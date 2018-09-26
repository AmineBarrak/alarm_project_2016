/* 3g.c
 *
 *
 * Copyright (c)2015 comelit R&D Tunisia
 *
 */ 
#include "3g_cred.h"
#include "uart_cred.h"
#include "ioexpander_cred.h"
#include "middleware_cred.h"
#include "notification_cred.h" 
#include "telephony_cred.h"
#include "dtmf_cred.h"
#include <stdarg.h>
#include <errno.h>

int uart_fd_global = -1;
static pthread_mutex_t  m3g_uartrx_mutex_global;
static pthread_t m3g_uartrx_pthread_global;
static pthread_t m3g_parse_pthread_global;
static unsigned char m3g_uartpoll_run=0;
static char m3g_rx_msg_event[5000];
static sem_t m3g_parsing_sem;
static enum m3g_pin_state  pin_state_global;
static char m3g_rx_msg_global[10000];
static int tcp_send_state_global=0;
static unsigned int m3g_tcp_connections_status_global[12];
static unsigned int fileupload_state_global ;
static char qsmpt_rx_global[20];
static char qsmpt_state_global;
static char connect_state_global;
static int http_get_state_global=0; 
static unsigned int read_size_global=0;	
static unsigned int debug_global=0;
static char contextid3_ipaddress[30];
static char contextid2_ipaddress[30];
static char contextid1_ipaddress[30]; 
static char m3g_tcp_in_msg_global[1000];
static unsigned int m3g_tcp_packet_is_got_global[12];
static int dtmf_fd[2];
static Dial_Response Dial_end_state_global=UNKNOWN_ERROR; 
static enum m3g_char_encode m3g_current_encode_idx;

static char*char_encode_list[4]={"GSM","IRA","HEX","UCS2"};

CRED_MW_Errors_t (*M3G_TS_Notify)(struct Ns_event_Str TsEvent); 
CRED_Errors_t M3G_UCS2ToUTF8(char * ucs2_msg,unsigned char* utf8);
CRED_Errors_t M3G_EventsProcessing(char*rx_msg);
CRED_Errors_t M3G_GetPinState(enum m3g_pin_state*pin_state); 
CRED_Errors_t M3G_AppendFileToRam(char*file_name,size_t file_size,char* file_buffer,int file_idx);
CRED_Errors_t M3G_ReadFile(char*file_name,size_t* file_size,char* file_buffer); 
CRED_Errors_t M3G_DataGetIPAddr();

CRED_Errors_t M3G_ExtractIntegers(char*str,uint32_t*pnum,uint32_t*psize)
{   
	char*rx_msg_pt=str; 
	uint32_t xcounter; 
	CRED_Errors_t err=CRED_NO_ERROR;
	
	if(str==NULL || pnum==NULL || psize==NULL)
	{
		M3G_TRACE_ERROR("null pointer was passed to %s\n",__FUNCTION__);
		return CRED_ERROR_BAD_PARAMETER;
	}   
	xcounter=0; 
	while(1)
	{ 
		while((rx_msg_pt[0]<48 || rx_msg_pt[0]>57) && rx_msg_pt[0]!='\0' && rx_msg_pt[0]!=NULL)  rx_msg_pt++;
		if(rx_msg_pt[0] =='\0') return err;
		if(rx_msg_pt[0] ==NULL) return err;
		pnum[xcounter]=atoi(rx_msg_pt);  
		xcounter++; 
		*psize=xcounter;
		while(rx_msg_pt[0]>47 && rx_msg_pt[0]<58 && rx_msg_pt[0]!='\0' && rx_msg_pt[0]!=NULL)  rx_msg_pt++;
		if(rx_msg_pt[0] =='\0') return err; 
		if(rx_msg_pt[0] ==NULL) return err;
	}  
	return err; 
}

void*M3G_RxParseThread(void*arg)
{
	M3G_TRACE("M3G_RxParse was added\n");
	while(1)
	{
		sem_wait(&m3g_parsing_sem);
		M3G_EventsProcessing(m3g_rx_msg_event);
		memset(m3g_rx_msg_event,0x00,sizeof(m3g_rx_msg_event));
	}
}

CRED_Errors_t M3G_RxParseEvents(char * rx_msg)
{
	CRED_Errors_t err = CRED_NO_ERROR; 
	
		
	if(rx_msg==NULL)
	{
		M3G_TRACE_ERROR("null pointer was passed to %s\n",__FUNCTION__);
		return CRED_ERROR_BAD_PARAMETER;
	} 
	
	memset(m3g_rx_msg_event,0x00,sizeof(m3g_rx_msg_event));
	strcpy(m3g_rx_msg_event,rx_msg); 
	sem_post(&m3g_parsing_sem);
	return err;
}

CRED_Errors_t M3G_RxReadOnce(char*rx_msg,unsigned int tim_out)
{
	CRED_Errors_t err = CRED_NO_ERROR; 
	unsigned int 	char_counter=0;  
	struct 			pollfd uart_fds;
	unsigned int 	read_size=0; 
		
	uart_fds.events = POLLIN;
	uart_fds.fd=uart_fd_global;
	
	UART_Read(uart_fd_global,rx_msg+char_counter,1500,&read_size);  
	char_counter+=read_size;
	while(1)
	{
		if(rx_msg[strlen(rx_msg)-1] == 0xa  && rx_msg[strlen(rx_msg)-2] == 0xd) 
		{
			break;
		}
		if(rx_msg[strlen(rx_msg)-1] == '>' || rx_msg[strlen(rx_msg)-2] == '>')
		{
			break;
		}
		int pollres=poll(&uart_fds,1, tim_out); 
		if( pollres == 0)
		{
			M3G_TRACE("M3G_RxReadOnce Timout\n"); 
			return CRED_ERROR_UNKNOWN;
		}
		else if( pollres < 0)
		{
			M3G_TRACE("M3G_RxReadOnce error occur when retrieving response\n"); 
			return CRED_ERROR_UNKNOWN;
		}
		UART_Read(uart_fd_global,rx_msg+char_counter,1500,&read_size);  
		char_counter+=read_size;
		read_size_global=char_counter;
	} 
	return CRED_NO_ERROR;
}

CRED_Errors_t M3G_RxReadUntilGetStr(char*rx_msg,char*str_to_got)
{
	CRED_Errors_t err = CRED_NO_ERROR; 
	unsigned int 	char_counter=0;  
	struct 			pollfd uart_fds;
	unsigned int 	read_size=0; 
	int pollres;
			
	if(rx_msg==NULL || str_to_got==NULL)
	{
		M3G_TRACE_ERROR("null pointer \n");
		return CRED_ERROR_BAD_PARAMETER;
	} 	
	uart_fds.events = POLLIN;
	uart_fds.fd=uart_fd_global;
	  
	while(1)
	{
		
		if(strstr(rx_msg,str_to_got)!=NULL && rx_msg[strlen(rx_msg)-1] == 0xa  && rx_msg[strlen(rx_msg)-2] == 0xd) 
		{
			break;
		}
		if(strstr(rx_msg,"+CMS ERROR：")!=NULL ||   strstr(rx_msg,"+CME ERROR："))
		{
			M3G_TRACE_ERROR("%s\n",rx_msg);
			return CRED_ERROR_UNKNOWN;
		}
		pollres=poll(&uart_fds,1,4000); 
		if( pollres == 0)
		{
			M3G_TRACE("M3G_RxReadUntilGetStr Timout\n"); 
			return CRED_ERROR_UNKNOWN;
		}
		else if( pollres < 0)
		{
			M3G_TRACE("M3G_RxReadUntilGetStr error occur when retrieving response\n"); 
			return CRED_ERROR_UNKNOWN;
		}
		UART_Read(uart_fd_global,rx_msg+char_counter,1500,&read_size);  
		char_counter+=read_size; 
	} 
	return CRED_NO_ERROR;
}

void* M3G_UARTPoll(void*arg)
{ 
	
	char 			rx_msg[5000]; 
	struct 			pollfd uart_fds; 
	int trylock_res;
	
	uart_fds.events = POLLIN;
	uart_fds.fd=uart_fd_global;   
	M3G_TRACE("M3G_UARTPoll added...\n");
	m3g_uartpoll_run=1;
	 
	while(m3g_uartpoll_run)
	{  
		memset(rx_msg,0x00,sizeof(rx_msg)); 
		//M3G_TRACE("--------M3G_UARTPoll start poll----------\n");
		poll(&uart_fds,1, -1);  
		//M3G_TRACE("--------M3G_UARTPoll end poll----------\n");
		trylock_res=pthread_mutex_trylock(&m3g_uartrx_mutex_global);
		if (trylock_res == 0)
		{ 
			//M3G_TRACE("--------M3G_UARTPoll trylock----------\n");
			M3G_RxReadOnce(rx_msg,100);
			//M3G_TRACE("Receiving1:\n%s",rx_msg);  
			if(strlen(rx_msg)>2) 
			{
				M3G_RxParseEvents(rx_msg);
			}
			pthread_mutex_unlock (&m3g_uartrx_mutex_global);
		}
	}
	pthread_exit(0);
}

CRED_Errors_t M3G_SendATCheck(unsigned int timout,char*format,...)			
{ 
	char buff_send[1000];     
	va_list args; 
	CRED_Errors_t err = CRED_NO_ERROR;  
	char 			rx_msg[1000]; 
	struct 			pollfd uart_fds; 
	int pollres;
	
	uart_fds.events = POLLIN;
	uart_fds.fd=uart_fd_global;   
	va_start(args, format);
	vsprintf(buff_send,format,args);
    va_end(args); 
	
    M3G_TRACE("Sending:%s",buff_send);  
    pthread_mutex_lock(&m3g_uartrx_mutex_global);
    UART_Write(uart_fd_global,buff_send, strlen(buff_send));
    
	if(debug_global) M3G_TRACE("starting polling\n");
    pollres=poll(&uart_fds,1, timout);
	if( pollres == 0)
	{
		M3G_TRACE("M3G_SendATCheck Timout\n");
		pthread_mutex_unlock (&m3g_uartrx_mutex_global);
		return CRED_ERROR_UNKNOWN;
	}
	else if( pollres < 0)
	{
		M3G_TRACE_ERROR("M3G_SendATCheck1 error occur when retrieving response1\n");
		pthread_mutex_unlock (&m3g_uartrx_mutex_global);
		return CRED_ERROR_UNKNOWN;
	}
	if(debug_global) M3G_TRACE("end polling\n");
	while(1)
	{ 
		memset(rx_msg,0x00,sizeof(rx_msg));
		if(debug_global) M3G_TRACE("Read a once time\n"); 
		if(CRED_NO_ERROR!=M3G_RxReadOnce(rx_msg,timout))
		{
			M3G_TRACE("M3G_SendATCheck2 error occur when retrieving response\n");
			pthread_mutex_unlock (&m3g_uartrx_mutex_global);
			return CRED_ERROR_UNKNOWN; 	
		}
		if(debug_global) M3G_TRACE("checking if OK\n");
		if(strstr(rx_msg,"OK") || strstr(rx_msg,"ERROR")) 
		{
			break;
		} 
		else if(strstr(rx_msg,"AT+"))
		{
			if(debug_global) M3G_TRACE("checking if AT\n");
			while(1)
			{
				if(strchr(rx_msg,'>'))
				{
					M3G_TRACE("you can start writing msg\n");
					pthread_mutex_unlock (&m3g_uartrx_mutex_global);
					return CRED_NO_ERROR;
				}
				if(strstr(rx_msg,"OK") || strstr(rx_msg,"CME") || strstr(rx_msg,"CMS")) 
				{
					break;
				} 
				if(CRED_NO_ERROR!=M3G_RxReadOnce(rx_msg+strlen(rx_msg),timout))
				{
					M3G_TRACE("error occur when retrieving response2\n");
					pthread_mutex_unlock (&m3g_uartrx_mutex_global);
					return CRED_ERROR_UNKNOWN; 	
				}
			}
			if(debug_global) M3G_TRACE("Receiving Internal AT response:\n%s",rx_msg); 
			break;
		}
		else
		{
			//M3G_TRACE("Receiving External events :\n%s",rx_msg);
			if(strlen(rx_msg)>2) 
			{
				M3G_RxParseEvents(rx_msg);
			}
		}
	}
	pthread_mutex_unlock (&m3g_uartrx_mutex_global);
	memset(m3g_rx_msg_global,0x00,sizeof(m3g_rx_msg_global));
	strcpy(m3g_rx_msg_global,rx_msg);
	m3g_rx_msg_global[strlen(m3g_rx_msg_global)]='\0';
	usleep(50000);
	if(strstr(rx_msg,"OK"))
	{
		return CRED_NO_ERROR;
	}
	else
	{
		M3G_TRACE_ERROR("Error AT rx_msg=%s\n",rx_msg);
		return CRED_ERROR_UNKNOWN;
	}
}

CRED_Errors_t M3G_SendAT(unsigned int timout,char*format,...)			
{ 
	char buff_send[200];     
	va_list args; 
	CRED_Errors_t err = CRED_NO_ERROR;   

	va_start(args, format);
	vsprintf(buff_send,format,args);
	va_end(args); 
	M3G_TRACE("Sending:%s",buff_send);  
	pthread_mutex_lock(&m3g_uartrx_mutex_global);
	UART_Write(uart_fd_global,buff_send, strlen(buff_send)); 
	pthread_mutex_unlock (&m3g_uartrx_mutex_global);  
	return err;
}

CRED_Errors_t M3G_SendATNoCheck(unsigned int timout,char*format,...)			
{ 
	char buff_send[200];     
	va_list args; 
	CRED_Errors_t err = CRED_NO_ERROR;   

	va_start(args, format);
	vsprintf(buff_send,format,args);
	va_end(args); 
	M3G_TRACE("Sending:%s",buff_send);  
	UART_Write(uart_fd_global,buff_send, strlen(buff_send)); 
	return err;
}
 
CRED_Errors_t M3G_Signal_Quality(Signal_Quality *Sig_Quality)
{ 
	CRED_Errors_t err = CRED_NO_ERROR;
	unsigned int numbers[5];
	unsigned int sizenum;
	char*rx_msg_pt;
	
	if(Sig_Quality==NULL)
	{
		M3G_TRACE_ERROR("Null pointer \n");
		return CRED_ERROR_BAD_PARAMETER; 
	}
	err=M3G_SendATCheck(1000,"AT+CSQ\r\n");
	if(CRED_NO_ERROR!=err)
	{
		M3G_TRACE_ERROR("cannot fetch signal quality\n"); 
		return CRED_ERROR_UNKNOWN;
	}  
	rx_msg_pt=strstr(m3g_rx_msg_global,"+CSQ:");
	if(rx_msg_pt == NULL)
	{
		M3G_TRACE_ERROR("cannot find +CSQ:\n"); 
		return CRED_ERROR_UNKNOWN; 
	} 
	M3G_ExtractIntegers(rx_msg_pt,numbers,&sizenum) ;  
	Sig_Quality->RSSI = numbers[0]; 
	Sig_Quality->BER = numbers[1];  
	M3G_TRACE("Sig_Quality->RSSI %d \n",Sig_Quality->RSSI);
	M3G_TRACE("Sig_Quality->BER %d \n",Sig_Quality->BER);
	
	return err;
}

CRED_Errors_t M3G_Sim_Network_Registration_Status(bool *Network_RegStatus)
{ 
	CRED_Errors_t err = CRED_NO_ERROR;
	unsigned int numbers[5];
	unsigned int sizenum;
	char*rx_msg_pt;
	
	if(Network_RegStatus==NULL)
	{
		M3G_TRACE_ERROR("Null pointer \n");
		return CRED_ERROR_BAD_PARAMETER; 
	}
	err=M3G_SendATCheck(1000,"AT+CREG?\r\n");
	if(CRED_NO_ERROR!=err)
	{
		M3G_TRACE_ERROR("cannot fetch gsm registration state\n"); 
		return CRED_ERROR_UNKNOWN;
	}  
	rx_msg_pt=strstr(m3g_rx_msg_global,"+CREG:");
	if(rx_msg_pt == NULL)
	{
		M3G_TRACE_ERROR("cannot find +CREG:\n"); 
		return CRED_ERROR_UNKNOWN; 
	}
	
	M3G_ExtractIntegers(rx_msg_pt,numbers,&sizenum) ; 
	*Network_RegStatus  =false;
	if(numbers[1]==0)
	{
		M3G_TRACE("gsm registration state:Not registered, ME is not currently searching a new operator to register to\n");
	}  
	else if(numbers[1]==1)
	{
		M3G_TRACE("gsm registration state:Registered, home network\n");
		*Network_RegStatus  =true;
	}  
	else if(numbers[1]==2)
	{
		M3G_TRACE("gsm registration state:Not registered, but ME is currently searching a new operator to register to\n");
	}  
	else if(numbers[1]==3)
	{
		M3G_TRACE("gsm registration state:Registration denied\n");
	}  
	else if(numbers[1]==4)
	{
		M3G_TRACE("gsm registration state:Unknown\n");
	}  
	else if(numbers[1]==5)
	{
		M3G_TRACE("gsm registration state:Registered, roaming\n");
	}    
	return err;
}


CRED_Errors_t M3G_Pin_Insert(char *PIN)
{
	CRED_Errors_t err=CRED_NO_ERROR; 
	unsigned int pin_state;
	
	if(PIN==NULL)
	{
		M3G_TRACE_ERROR("NULL pointer\n");
		return CRED_ERROR_BAD_PARAMETER;
	}
	M3G_GetPinState(&pin_state);
	if(SIM_PIN_PUK_OK== pin_state)
	{
		M3G_TRACE("PIN is already inserted\n");
	}
	err=M3G_SendATCheck(1000,"AT+CPIN=%s\r\n",PIN); 
	return err;
}

CRED_Errors_t M3G_PukPinInsert(char*puk,char*new_pin)
{
	CRED_Errors_t err=CRED_NO_ERROR;
	
	if(puk==NULL || new_pin==NULL)
	{
		M3G_TRACE_ERROR("NULL pointer\n");
		return CRED_ERROR_BAD_PARAMETER;
	} 
	err=M3G_SendATCheck(1000,"AT+CPIN=\"%s\",\"%s\"\r\n",puk,new_pin);  
	return err;
}

CRED_Errors_t M3G_Pin_Set_New(char *PIN_Old, char *PIN_New)
{
	CRED_Errors_t err=CRED_NO_ERROR;
	
	if(PIN_Old==NULL || PIN_Old==NULL)
	{
		M3G_TRACE_ERROR("NULL pointer\n");
		return CRED_ERROR_BAD_PARAMETER;
	} 
	err=M3G_SendATCheck(1000,"AT+CLCK=\"SC\",2\r\n"); 
	if(strstr(m3g_rx_msg_global,"+CLCK: 1")==NULL)
	{
		M3G_TRACE_ERROR("pin must locked first\n");
		return CRED_ERROR_UNKNOWN;
	}
	err=M3G_SendATCheck(1000,"AT+CPWD=\"SC\",\"%s\",\"%s\"\r\n",PIN_Old,PIN_New);  
	return err;
}

CRED_Errors_t M3G_Pin_Lock(char *PIN)
{
	CRED_Errors_t err=CRED_NO_ERROR;
	if(PIN==NULL)
	{
		M3G_TRACE_ERROR("NULL pointer\n");
		return CRED_ERROR_BAD_PARAMETER;
	}
	err=M3G_SendATCheck(1000,"AT+CLCK=\"SC\",1,\"%s\"\r\n",PIN);  
	return err;
}

CRED_Errors_t M3G_Pin_Unlock(char *PIN)
{
	CRED_Errors_t err=CRED_NO_ERROR;
	if(PIN==NULL)
	{
		M3G_TRACE_ERROR("NULL pointer\n");
		return CRED_ERROR_BAD_PARAMETER;
	}
	err=M3G_SendATCheck(1000,"AT+CLCK=\"SC\",0,\"%s\"\r\n",PIN);  
	return err;
}

CRED_Errors_t M3G_Check_MODULE_Init_Status(bool *Init_Status)
{  
	CRED_Errors_t err = CRED_NO_ERROR;
	unsigned int numbers[5];
	unsigned int sizenum;
	
	if(Init_Status==NULL)
	{
		M3G_TRACE_ERROR("NULL pointer\n");
		return CRED_ERROR_BAD_PARAMETER;
	}
	err=M3G_SendATCheck(1000,"AT+QINISTAT\r\n");
	if(CRED_NO_ERROR!=err)
	{
		M3G_TRACE_ERROR("cannot fetch m3g init state\n"); 
		return CRED_ERROR_UNKNOWN;
	}  
	char*rx_msg_pt=strstr(m3g_rx_msg_global,"+QINISTAT:");
	if(rx_msg_pt == NULL)
	{
		M3G_TRACE_ERROR("cannot find +QINISTAT:\n"); 
		return CRED_ERROR_UNKNOWN; 
	}
	*Init_Status=false;
	M3G_ExtractIntegers(rx_msg_pt,numbers,&sizenum) ; 
	if(numbers[0] == 0)
	{
		M3G_TRACE("No initialization\n");
	}
	else if(numbers[0] == 1)
	{
		M3G_TRACE("Ready to execute AT command\n");
	}
	else if(numbers[0] == 2)
	{
		M3G_TRACE("Phonebook has finished initialization\n");
	}  
	else if(numbers[0] == 3)
	{
		M3G_TRACE("SMS has finished initialization\n");
		*Init_Status=true;
	}  
	return CRED_NO_ERROR; 
}

CRED_Errors_t M3G_Module_Information(Module_Info *Mod_Info)
{
	CRED_Errors_t err = CRED_NO_ERROR;
	unsigned int numbers[5];
	unsigned int sizenum;
	char*rx_msg_pt;
	char*rx_msg_pt1;
	
	if(Mod_Info==NULL)
	{
		M3G_TRACE_ERROR("Null pointer \n");
		return CRED_ERROR_BAD_PARAMETER; 
	}
	memset(Mod_Info,0x00,sizeof(Module_Info)); 
	err=M3G_SendATCheck(1000,"ATI\r\n");
	if(CRED_NO_ERROR!=err)
	{
		M3G_TRACE_ERROR("cannot fetch Module Information\n"); 
		return CRED_ERROR_UNKNOWN;
	}  
	rx_msg_pt=strstr(m3g_rx_msg_global,"ATI");
	if(rx_msg_pt==NULL)
	{
		M3G_TRACE_ERROR("cannot find ATI\n"); 
		return CRED_ERROR_UNKNOWN;
	}
	rx_msg_pt=1+strchr(rx_msg_pt,'\n');
	rx_msg_pt1=strchr(rx_msg_pt,'\n');  
	strncpy(Mod_Info->Manufacturer_ID,rx_msg_pt,rx_msg_pt1-rx_msg_pt); 
	Mod_Info->Manufacturer_ID[strlen(Mod_Info->Manufacturer_ID)]='\0';
	M3G_TRACE("MMod_Info->Manufacturer_ID:%s \n",Mod_Info->Manufacturer_ID);   
	
	rx_msg_pt1++;
	rx_msg_pt=strchr(rx_msg_pt1,'\n'); 
	strncpy(Mod_Info->Device_module,rx_msg_pt1,rx_msg_pt-rx_msg_pt1); 
	Mod_Info->Device_module[strlen(Mod_Info->Device_module)]='\0';
	M3G_TRACE("MMod_Info->Device_module:%s \n",Mod_Info->Device_module); 
	
	rx_msg_pt=strstr(m3g_rx_msg_global,"Revision: ")+strlen("Revision: "); 
	rx_msg_pt1=strchr(rx_msg_pt,'\n');
	strncpy(Mod_Info->FW_Version,rx_msg_pt,rx_msg_pt1-rx_msg_pt-1);    
	Mod_Info->FW_Version[strlen(Mod_Info->FW_Version)]='\0';
	M3G_TRACE("Mod_Info->FW_Version:%s \n",Mod_Info->FW_Version);   
	
	err=M3G_SendATCheck(1000,"AT+GSN\r\n");
	if(CRED_NO_ERROR!=err)
	{
		M3G_TRACE_ERROR("cannot fetch IMEI\n"); 
		return CRED_ERROR_UNKNOWN;
	}  
	rx_msg_pt=strstr(m3g_rx_msg_global,"AT+GSN");
	if(rx_msg_pt==NULL)
	{
		M3G_TRACE_ERROR("cannot find AT+GSN\n"); 
		return CRED_ERROR_UNKNOWN;
	}
	rx_msg_pt=1+strchr(rx_msg_pt,'\n');
	rx_msg_pt1=strchr(rx_msg_pt,'\n'); 
	strncpy(Mod_Info->Device_IMEI,rx_msg_pt,rx_msg_pt1-rx_msg_pt-1);  
	M3G_TRACE("Mod_Info->Device_IMEI:%s \n",Mod_Info->Device_IMEI);    
	return err;
}

CRED_Errors_t M3G_Registration_Information(Registration_Info *Reg_Info)
{
	CRED_Errors_t err = CRED_NO_ERROR;
	unsigned int numbers[5];
	unsigned int sizenum;
	char*rx_msg_pt;
	char*rx_msg_pt2;
	
	if(Reg_Info==NULL)
	{
		M3G_TRACE_ERROR("Null pointer \n");
		return CRED_ERROR_BAD_PARAMETER; 
	}
	memset(Reg_Info , 0x00 , sizeof(Registration_Info)); 
	err=M3G_SendATCheck(1000,"AT+COPS?\r\n");
	if(CRED_NO_ERROR!=err)
	{
		M3G_TRACE_ERROR("cannot fetch operator name\n"); 
		return CRED_ERROR_UNKNOWN;
	}  
	rx_msg_pt=strstr(m3g_rx_msg_global,"+COPS:");
	if(rx_msg_pt == NULL)
	{
		M3G_TRACE_ERROR("cannot find +COPS:\n"); 
		return CRED_ERROR_UNKNOWN; 
	} 
	M3G_ExtractIntegers(rx_msg_pt,numbers,&sizenum) ;  
	rx_msg_pt=strchr(rx_msg_pt,'"');
	if(rx_msg_pt == NULL)
	{
		M3G_TRACE_ERROR("cannot find frist \"\n"); 
		return CRED_ERROR_UNKNOWN; 
	} 
	rx_msg_pt2=strchr(rx_msg_pt+1,'"');
	if(rx_msg_pt2 == NULL)
	{
		M3G_TRACE_ERROR("cannot find second \"\n"); 
		return CRED_ERROR_UNKNOWN; 
	} 
	strncpy(Reg_Info->Operator,rx_msg_pt+1,rx_msg_pt2-rx_msg_pt-1); 
	M3G_TRACE("Operator name :%s\n",Reg_Info->Operator);
	 
	switch(numbers[2])
	{
		case 0:
		{
			Reg_Info->Tech = GSM;
			M3G_TRACE("current technology selected : %d\n", Reg_Info->Tech);
		}
		break;	
		case 2:
		{
			Reg_Info->Tech = UTRAN;
			M3G_TRACE("current technology selected : %d\n", Reg_Info->Tech);
		}
		break;	
		case 3:
		{
			Reg_Info->Tech = GSM_W_EGPRS;
			M3G_TRACE("current technology selected : %d\n", Reg_Info->Tech);
		}
		break;	
		case 4:
		{
			Reg_Info->Tech = UTRAN_W_HSDPA;
			M3G_TRACE("current technology selected : %d\n", Reg_Info->Tech);
		}
		break;	
		case 5:
		{
			Reg_Info->Tech = UTRAN_W_HSUPA;
			M3G_TRACE("current technology selected : %d\n", Reg_Info->Tech);
		}
		break;	
		case 6:
		{
			Reg_Info->Tech = UTRAN_W_HSDPA_and_HSUPA;
			M3G_TRACE("current technology selected : %d\n", Reg_Info->Tech);
		}
		break;	
		default:
		{
			Reg_Info->Tech = UNKNOWN_TECHNOLOGIE;
			M3G_TRACE("there are a problem when detecting current technologie : %d\n", Reg_Info->Tech);
		}
		break;	
	} 

	return err;
}

CRED_Errors_t M3G_Sim_Card_Information(Sim_Card_Info *Sim_Info)
{
	CRED_Errors_t err = CRED_NO_ERROR; 
	char*rx_msg_pt;
	char*rx_msg_pt1;
	
	if(Sim_Info==NULL)
	{
		M3G_TRACE_ERROR("Null pointer \n");
		return CRED_ERROR_BAD_PARAMETER; 
	}
	memset( Sim_Info,0x00,sizeof(Sim_Card_Info));
	
	err=M3G_SendATCheck(1000,"AT+CIMI\r\n");
	if(CRED_NO_ERROR!=err)
	{
		M3G_TRACE_ERROR("cannot fetch sim Information\n"); 
		return CRED_ERROR_UNKNOWN;
	} 
	rx_msg_pt=strstr(m3g_rx_msg_global,"AT+CIMI");
	if(rx_msg_pt==NULL)
	{
		M3G_TRACE_ERROR("cannot find AT+CIMI\n"); 
		return CRED_ERROR_UNKNOWN;
	}
	rx_msg_pt=strchr(rx_msg_pt,'\n');  
	rx_msg_pt+=1;
	rx_msg_pt1=strchr(rx_msg_pt,'\n'); 
	strncpy(Sim_Info->IMSI ,rx_msg_pt,rx_msg_pt1-rx_msg_pt-1);
	M3G_TRACE("Sim_Info->IMSI:%s \n",Sim_Info->IMSI);
	
	err=M3G_SendATCheck(1000,"AT+QCCID\r\n");
	if(CRED_NO_ERROR!=err)
	{
		M3G_TRACE_ERROR("cannot fetch sim Information\n"); 
		return CRED_ERROR_UNKNOWN;
	} 
	rx_msg_pt=strstr(m3g_rx_msg_global,"+QCCID:");
	if(rx_msg_pt==NULL)
	{
		M3G_TRACE_ERROR("cannot find +QCCID:\n"); 
		return CRED_ERROR_UNKNOWN;
	}
	rx_msg_pt +=8;  
	rx_msg_pt1=strchr(rx_msg_pt,'\n'); 
	strncpy(Sim_Info->ICCID ,rx_msg_pt,rx_msg_pt1-rx_msg_pt-1);
	M3G_TRACE("Sim_Info->ICCID:%s \n",Sim_Info->ICCID);
	return err;
}

CRED_Errors_t M3G_Search_Networks(Networks_Available *Networks)
{
	CRED_Errors_t err = CRED_NO_ERROR; 
	M3G_SendAT(40000,"AT+COPS=?\r\n"); 
	return err;
}

CRED_Errors_t M3G_Manual_Disconnect_Network()
{
	CRED_Errors_t err = CRED_NO_ERROR; 
	
	
	return CRED_NO_ERROR;
}

CRED_Errors_t M3G_Manual_Connect_Network(int fd, Network_Info *network )
{
	CRED_Errors_t err = CRED_NO_ERROR;

	return err;
}

CRED_Errors_t M3G_Auto_Connect_Network(int fd)
{
	CRED_Errors_t err = CRED_NO_ERROR;
	
	return CRED_NO_ERROR;
}
	
CRED_Errors_t M3G_Check_Call_Response(Dial_Response *Dial)
{ 
	CRED_Errors_t err = CRED_NO_ERROR;
	 
	if (Dial == NULL)
	{
		M3G_TRACE_ERROR("NULL Dial Pointer\n");
		return CRED_ERROR_BAD_PARAMETER;	
	} 
	*Dial = Dial_end_state_global;  
 
	return CRED_NO_ERROR;
}

CRED_Errors_t M3G_GetPinState(enum m3g_pin_state*pin_state)
{
	CRED_Errors_t err = CRED_NO_ERROR;
				
	if(pin_state==NULL)
	{
		M3G_TRACE_ERROR("null pointer \n");
		return CRED_ERROR_BAD_PARAMETER;
	}
	err=M3G_SendATCheck(1000,"AT+CPIN?\r\n");
	if(CRED_NO_ERROR!=err)
	{
		M3G_TRACE_ERROR("cannot fetch Pin state\n"); 
		return CRED_ERROR_UNKNOWN;
	}  
	if(NULL!=strstr(m3g_rx_msg_global,"READY")) 
	{
		M3G_TRACE("PIN is ready\n");
		*pin_state=SIM_PIN_PUK_OK;
	}
	else if(NULL!=strstr(m3g_rx_msg_global,"SIM PIN")) 
	{
		M3G_TRACE("ME is waiting for SIM PIN\n");  
		*pin_state=SIM_PIN_REQUIRED;
	}      
	else if(NULL!=strstr(m3g_rx_msg_global,"SIM PUK")) 
	{
		M3G_TRACE("ME is waiting for SIM PUK\n");  
		*pin_state=SIM_PUK_REQUIRED;
	}  
	pin_state_global=*pin_state;
	return CRED_NO_ERROR;
}

CRED_Errors_t M3G_check_sim_status(bool*sim_status)  
{
	CRED_Errors_t err = CRED_NO_ERROR;
	unsigned int numbers[5];
	unsigned int sizenum;
	char*rx_msg_pt;
				
	if(sim_status==NULL)
	{
		M3G_TRACE_ERROR("null pointer \n");
		return CRED_ERROR_BAD_PARAMETER;
	}
	err=M3G_SendATCheck(1000,"AT+QSIMSTAT?\r\n");
	if(CRED_NO_ERROR!=err)
	{
		M3G_TRACE_ERROR("cannot fetch SIM state\n"); 
		return CRED_ERROR_UNKNOWN;
	}  
	rx_msg_pt=strstr(m3g_rx_msg_global,"+QSIMSTAT:");
	if(rx_msg_pt ==NULL)
	{
		M3G_TRACE_ERROR("cannot find +QSIMSTAT:\n"); 
		return CRED_ERROR_UNKNOWN; 
	}
	
	*sim_status=false;
	M3G_ExtractIntegers(rx_msg_pt,numbers,&sizenum) ;
	if(numbers[1]== 0)
	{
		M3G_TRACE("SIM Removed\n");
	}
	else if(numbers[1]== 1)
	{
		*sim_status=true;
		M3G_TRACE("SIM Inserted\n");
	}
	else if(numbers[1]== 2)
	{
		M3G_TRACE("Unknown SIM state\n");
	}  
	return CRED_NO_ERROR;
}

CRED_Errors_t M3G_Deny_Call( )
{ 
	CRED_Errors_t err = CRED_NO_ERROR; 
		
	M3G_SendATCheck(1000,"AT+CLCC\r\n");
	if(strstr(m3g_rx_msg_global,"+CLCC:") == NULL)
	{
		M3G_TRACE_ERROR("No call found at present\n");
		return  CRED_NO_ERROR;
	}  
	M3G_TRACE("Rejecting....\n"); 
	//err=M3G_SendATCheck(1000,"AT+QHUP=21,1\r\n");
	err=M3G_SendATCheck(1000,"ATH\r\n");
	return err;
}

CRED_Errors_t M3G_Answer_Call( )

{
	CRED_Errors_t err = CRED_NO_ERROR; 
	char * strstrres;
	
	M3G_SendATCheck(1000,"AT+CLCC\r\n");
	if(strstr(m3g_rx_msg_global,"+CLCC:") == NULL )
	{
		M3G_TRACE_ERROR("No call found at present\n");
		return  CRED_NO_ERROR;
	}  
	M3G_TRACE("Accepting....\n");
	err=M3G_SendATCheck(1000,"ATA\r\n");
	return err;
}

CRED_Errors_t M3G_RxRead(char*rx_msg)
{
	CRED_Errors_t err = CRED_NO_ERROR;  
	struct 			pollfd uart_fds;
	unsigned int 	read_size=0; 
	int pollres;
		
	uart_fds.events = POLLIN;
	uart_fds.fd=uart_fd_global;  
	pollres=poll(&uart_fds,1, 1000); 
	if( pollres == 0)
	{
		M3G_TRACE_ERROR("Timout\n"); 
		return CRED_ERROR_UNKNOWN;
	}
	else if( pollres < 0)
	{
		M3G_TRACE_ERROR("error occur when retrieving response\n"); 
		return CRED_ERROR_UNKNOWN;
	}
	UART_Read(uart_fd_global,rx_msg,200,&read_size);  
	return CRED_NO_ERROR;
}

CRED_Errors_t M3G_Init_SMS( )
{
	CRED_Errors_t err = CRED_NO_ERROR; 
	char*rx_msg_pt;
	char*rx_msg_pt1;
	char sms_center[20];
	
	M3G_SendATCheck(1000,"AT+CMGF=1\r\n");  
	M3G_SelectCharEncod(M3G_CHAR_GSM);  
	M3G_SendATCheck(1000,"AT+CSMP=17,167,0,0\r\n");
	M3G_SendATCheck(1000,"AT+CPMS=\"SM\",\"SM\",\"SM\"\r\n");   
	err=M3G_SendATCheck(1000,"AT+CSCA?\r\n");
	if(CRED_NO_ERROR!=err)
	{
		M3G_TRACE_ERROR("cannot fetch sms center Informations\n"); 
		return CRED_ERROR_UNKNOWN;
	} 
	rx_msg_pt=strstr(m3g_rx_msg_global,"+CSCA:");
	if(rx_msg_pt == NULL )
	{  
		M3G_TRACE_ERROR("cannot find +CSCA:\n"); 
		return CRED_ERROR_UNKNOWN;
	}
	memset(sms_center,0x00,sizeof(sms_center));
	rx_msg_pt=strchr(rx_msg_pt,'"');
	rx_msg_pt1=strchr(rx_msg_pt+1,'"'); 
	strncpy(sms_center,rx_msg_pt+1,rx_msg_pt1-rx_msg_pt-1);
	M3G_TRACE("sms center :%s\n",sms_center);
	return err;
} 

CRED_Errors_t M3G_UTF8_UCS2(unsigned char * input,char*output)
{
	unsigned int inputlen;
	unsigned int oneucs2;
	char *inputstr;
	char *inputendstr;
	char *outputstr;
	char outbuff[5];
	CRED_Errors_t err = CRED_NO_ERROR; 
	
	if(input==NULL || output==NULL)
	{
		printf("NULL pointer\n");
		return CRED_ERROR_BAD_PARAMETER;
	}
	inputstr=input;
	inputendstr=input+strlen(input);
	outputstr=output;
	while(inputstr<inputendstr)
	{
		if(((unsigned char)inputstr[0])<127)
		{
			oneucs2=(unsigned char)inputstr[0];
			inputstr++;
		}
		else
		{
			oneucs2=(((unsigned char)inputstr[0] & 0x1F)<<6  |  ((unsigned char)inputstr[1] & 0x3F));
			inputstr+=2;
		}
		if(oneucs2>15)
		{
			memset(outbuff,0x00,sizeof(outbuff));
			
			if(oneucs2<256)
			{
				sprintf(outbuff,"00%x",oneucs2);
			}
			else if(oneucs2<4096)
			{
				sprintf(outbuff,"0%x",oneucs2);
			}
			else
			{
				sprintf(outbuff,"%x",oneucs2);
			}
			strcpy(outputstr,outbuff);
			outputstr+=4;
		}
	}
    return err;
}
CRED_Errors_t M3G_UCS2ToUTF8(char * ucs2_msg,unsigned char* utf8)
{
	unsigned char onehexchar[5];
	unsigned int ucs2_msg_len;
	unsigned int  convres;
	unsigned char* ucs2_msg_pt;
	unsigned char* utf8_pt;
	CRED_Errors_t err = CRED_NO_ERROR; 
	
	if(ucs2_msg == NULL || utf8==NULL )
	{
		printf("Null pointer\n");
		return CRED_ERROR_BAD_PARAMETER;
	} 	
	
	utf8_pt=utf8;
	ucs2_msg_pt=ucs2_msg;
	ucs2_msg_len=strlen(ucs2_msg);
	
	while(ucs2_msg_pt<ucs2_msg+ucs2_msg_len)
	{
		if(ucs2_msg_pt[0]!='0') ucs2_msg_pt++;
		if(ucs2_msg_pt[0]!='0') 
		{
			return err;
		}
		memset(onehexchar,0x00,sizeof(onehexchar));
		strncpy(onehexchar,ucs2_msg_pt,4);
		convres=strtol(onehexchar, NULL, 16);
		if(convres<127)
		{
			utf8_pt[0]=convres;
			utf8_pt++;
		}
		else
		{
			utf8_pt[0] = (convres >> 6)   | 0xC0;
			utf8_pt++;
			utf8_pt[0] = (convres & 0x3F) | 0x80;
			utf8_pt++;
		}
		ucs2_msg_pt+=4;
	}
	return err;
}

CRED_Errors_t M3G_UTF8_CharSet(char * input,char*output)
{
	if(input==NULL||output==NULL)
	{
		M3G_TRACE_ERROR("NULL pointer\n");
		return CRED_ERROR_BAD_PARAMETER;
	}
	switch(m3g_current_encode_idx)
	{
		case M3G_CHAR_GSM:
			M3G_TRACE("encoding to M3G_CHAR_GSM\n");
			strcpy(output,input);
		break;
		case M3G_CHAR_IRA:
			M3G_TRACE("encoding to M3G_CHAR_IRA\n");
			strcpy(output,input);
		break;
		case M3G_CHAR_HEX:
			M3G_TRACE("encoding to M3G_CHAR_HEX\n");
			strcpy(output,input);
		break;
		case M3G_CHAR_UCS2:
			M3G_TRACE("encoding to M3G_CHAR_UCS2\n");
			M3G_UTF8_UCS2(input,output);
		break;
		default:
		M3G_TRACE("choice out of range %d\n",m3g_current_encode_idx);
		break;
	}
	//M3G_TRACE("input:%s\n",input);
	//M3G_TRACE("output:%s\n",output);
	
}

CRED_Errors_t M3G_CharSet_UTF8(char * input,char*output)
{
	if(input==NULL||output==NULL)
	{
		M3G_TRACE_ERROR("NULL pointer\n");
		return CRED_ERROR_BAD_PARAMETER;
	}
	switch(m3g_current_encode_idx)
	{
		case M3G_CHAR_GSM:
			M3G_TRACE("decoding from M3G_CHAR_GSM\n");
			strcpy(output,input);
		break;
		case M3G_CHAR_IRA:
			M3G_TRACE("decoding from M3G_CHAR_IRA\n");
			strcpy(output,input);
		break;
		case M3G_CHAR_HEX:
			M3G_TRACE("decoding from M3G_CHAR_HEX\n");
			strcpy(output,input);
		break;
		case M3G_CHAR_UCS2:
			M3G_TRACE("decoding from M3G_CHAR_UCS2\n");
			M3G_UCS2ToUTF8(input,output);
		break;
		default:
		M3G_TRACE("choice out of range %d\n",m3g_current_encode_idx);
		break;
	}
	//M3G_TRACE("input:%s\n",input);
	//M3G_TRACE("output:%s\n",output);
}
CRED_Errors_t M3G_SelectCharEncod(enum m3g_char_encode encode_idx)
{ 
	CRED_Errors_t err = CRED_NO_ERROR; 
	
	if(encode_idx>M3G_CHAR_LAST-1)
	{
		M3G_TRACE_ERROR("invalid encoding type\n");
		return CRED_ERROR_BAD_PARAMETER;
	} 
	m3g_current_encode_idx=encode_idx;
	M3G_SendATCheck(2000,"AT+CSCS=\"%s\"\r\n",char_encode_list[encode_idx]);
	return err; 
}

CRED_Errors_t M3G_Send_SMS(char *num, char *msg)
{ 
	CRED_Errors_t err = CRED_NO_ERROR; 
    char sms_msg[2000];
    char sms_msg1[2000];
    char sms_num[200];
    char rx_msg[50];
    
    if(num==NULL || msg == NULL)
    {
		M3G_TRACE_ERROR("NULL pointer\n");
		return CRED_ERROR_BAD_PARAMETER;
	} 
	M3G_SelectCharEncod(M3G_CHAR_GSM);
	memset(sms_msg,0x00,sizeof(sms_msg));
	memset(sms_msg1,0x00,sizeof(sms_msg1));
	memset(sms_num,0x00,sizeof(sms_num));
	M3G_UTF8_CharSet(msg,sms_msg); 
	M3G_UTF8_CharSet(num,sms_num); 
	M3G_UCS2ToUTF8(sms_msg,sms_msg1);
	M3G_TRACE("codec:%s\n",sms_msg1);
	sms_msg[strlen(sms_msg)]='\x1A'; 
	err = M3G_SendATCheck(1000,"AT+CMGS=\"%s\"\r\n", sms_num); 
	pthread_mutex_lock(&m3g_uartrx_mutex_global);
	UART_Write(uart_fd_global, sms_msg,strlen(sms_msg));
	pthread_mutex_unlock(&m3g_uartrx_mutex_global); 
	M3G_SelectCharEncod(M3G_CHAR_UCS2);
	return err; 
}

CRED_Errors_t M3G_SMSList(sms_config**sms_config_list,unsigned int maxsize )
{ 	  
	CRED_Errors_t err = CRED_NO_ERROR; 
	char*rx_msg[100000];
	char*rx_msgpt;
	struct 			pollfd uart_fds;
	unsigned int 	read_size=0; 
	char*str1ptr ;
	char*str2ptr ;
	unsigned int sms_config_counter=0; 
	int pollres;
	char ira[400];
	
	if(sms_config_list == NULL)
	{
		M3G_TRACE_ERROR("Null pointer\n");
		return CRED_ERROR_BAD_PARAMETER;
	}   
	M3G_SelectCharEncod(M3G_CHAR_UCS2);
	M3G_TRACE("Gathering data list\n");
	str1ptr=(char*)rx_msg;
	uart_fds.events = POLLIN;
	uart_fds.fd=uart_fd_global;     
	rx_msgpt=(char*)rx_msg;  
	memset(rx_msg,0x00,sizeof(rx_msg));
	pthread_mutex_lock(&m3g_uartrx_mutex_global);
	UART_Write(uart_fd_global,"AT+CMGL=\"ALL\"\r\n",strlen("AT+CMGL=\"ALL\"\r\n")); 
	while(read_size<90000)
	{ 
		pollres=poll(&uart_fds,1,2000); 
		if( pollres == 0)
		{
			M3G_TRACE("Timout\n"); 
			break;
		}
		else if( pollres < 0)
		{
			M3G_TRACE_ERROR("error occur when retrieving response\n"); 
			break;
		} 
		UART_Read(uart_fd_global,rx_msgpt,200,&read_size);
		if((strstr(rx_msgpt,"OK") || strstr(rx_msgpt,"ERROR")) && strstr(rx_msgpt,"\r\n")) 
		{
			break;
		}
		rx_msgpt +=read_size; 
	}
	pthread_mutex_unlock(&m3g_uartrx_mutex_global);  
	rx_msgpt +=read_size; 
	rx_msgpt[0]='\0';
	str1ptr=(char*)rx_msg; 
	M3G_TRACE("data list is got\n");
	//M3G_TRACE("-----------------------------------------------\n");
	//M3G_TRACE("%s",str1ptr);
	//M3G_TRACE("-----------------------------------------------\n");  
	sms_config_counter=0;
	
	M3G_TRACE("parsing data list\n");
	sleep(2);
	while(sms_config_counter<maxsize)
	{   
		if((str1ptr=strstr(str1ptr,"+CMGL:"))==NULL)
		{
			M3G_TRACE("----------------------------------------------------\n");
			M3G_TRACE("cannot find +CMGL:\n");
			return err;
		}  
		if(sms_config_list[sms_config_counter]==NULL)
		{
			M3G_TRACE_ERROR("Null pointer line:%d\n",__LINE__);
			return CRED_ERROR_UNKNOWN;
		}  
		memset(sms_config_list[sms_config_counter],0x00,sizeof(sms_config));  
		M3G_TRACE("----------------------------------------------------\n"); 	
		str1ptr+=strlen("+CMGL: "); 
		sms_config_list[sms_config_counter]->index=atoi(str1ptr);
		M3G_TRACE("index :%d\n",sms_config_list[sms_config_counter]->index); 
		
		str1ptr=strchr(str1ptr,'"');
		if(str1ptr==NULL)
		{
			M3G_TRACE_ERROR("Null pointer line:%d\n",__LINE__);
			return CRED_ERROR_UNKNOWN;
		}
		str1ptr++;
		str2ptr=strchr(str1ptr,'"');
		if(str2ptr==NULL)
		{
			M3G_TRACE_ERROR("Null pointer line:%d\n",__LINE__);
			return CRED_ERROR_UNKNOWN;
		}
		strncpy(sms_config_list[sms_config_counter]->sms_type,str1ptr,str2ptr-str1ptr);
		M3G_TRACE("sms_type:%s\n",sms_config_list[sms_config_counter]->sms_type); 
		
		str1ptr=strchr(str1ptr,'"');
		if(str1ptr==NULL)
		{
			M3G_TRACE_ERROR("Null pointer line:%d\n",__LINE__);
			return CRED_ERROR_UNKNOWN;
		}
		str1ptr++;
		str1ptr=strchr(str1ptr,'"');
		if(str1ptr==NULL)
		{
			M3G_TRACE_ERROR("Null pointer line:%d\n",__LINE__);
			return CRED_ERROR_UNKNOWN;
		}
		str1ptr++;
		str2ptr=strchr(str1ptr,'"');
		if(str2ptr==NULL)
		{
			M3G_TRACE_ERROR("Null pointer line:%d\n",__LINE__);
			return CRED_ERROR_UNKNOWN;
		}
		memset(ira,0x00,sizeof(ira));
		strncpy(ira,str1ptr,str2ptr-str1ptr);
		M3G_CharSet_UTF8(ira,sms_config_list[sms_config_counter]->num);
		M3G_TRACE("num :%s\n",sms_config_list[sms_config_counter]->num);
		
		str1ptr=strchr(str1ptr,'"');
		if(str1ptr==NULL)
		{
			M3G_TRACE_ERROR("Null pointer line:%d\n",__LINE__);
			return CRED_ERROR_UNKNOWN;
		}
		str1ptr++;
		str1ptr=strchr(str1ptr,'"');
		if(str1ptr==NULL)
		{
			M3G_TRACE_ERROR("Null pointer line:%d\n",__LINE__);
			return CRED_ERROR_UNKNOWN;
		}
		str1ptr++;
		str2ptr=strchr(str1ptr,'"');
		if(str2ptr==NULL)
		{
			M3G_TRACE_ERROR("Null pointer line:%d\n",__LINE__);
			return CRED_ERROR_UNKNOWN;
		}
		strncpy(sms_config_list[sms_config_counter]->date,str1ptr,str2ptr-str1ptr); 
		M3G_TRACE("date:%s\n",sms_config_list[sms_config_counter]->date); 
		
		str1ptr=strchr(str1ptr,'\n');
		if(str1ptr==NULL)
		{
			M3G_TRACE_ERROR("Null pointer line:%d\n",__LINE__);
			return CRED_ERROR_UNKNOWN;
		} 
		str1ptr++;
		str2ptr=strchr(str1ptr,'\n'); 
		if(str2ptr==NULL)
		{
			M3G_TRACE_ERROR("Null pointer line:%d\n",__LINE__);
			return CRED_ERROR_UNKNOWN;
		} 
		memset(ira,0x00,sizeof(ira));
		strncpy(ira,str1ptr,str2ptr-str1ptr); 
		M3G_CharSet_UTF8(ira,sms_config_list[sms_config_counter]->msg);
			  	 	 	 
		M3G_TRACE("msg:%s\n",sms_config_list[sms_config_counter]->msg); 	 
		sms_config_counter++;
	}	
	
EXIT:  
	return err;
}

CRED_Errors_t M3G_Call_Number(char *num)
{ 
	CRED_Errors_t err = CRED_NO_ERROR;  
	
	if(num==NULL)
	{
		M3G_TRACE_ERROR("Null pointer\n");
		return CRED_ERROR_BAD_PARAMETER;
	}
	M3G_TRACE("start calling....%s\n",num);
	err=M3G_SendATCheck(1000,"ATD%s;\r\n",num);    
	return err;
}

CRED_Errors_t M3G_MailClean( )
{  
	CRED_Errors_t err=CRED_NO_ERROR;
	M3G_SendATCheck(1000,"AT+QSMTPCLR\r\n");  
	err=M3G_SendATCheck(1000,"AT+QFDEL=\"RAM:*\"\r\n" );       
	return err;   
} 

CRED_Errors_t M3G_MMSConfig(struct mms_config * mms_configpt)
{ 
	CRED_Errors_t err = CRED_NO_ERROR; 
   
	if(mms_configpt == NULL)
	{
		M3G_TRACE_ERROR("Null pointer\n");
		return CRED_ERROR_BAD_PARAMETER;
	}   
	M3G_SendATCheck(1000,"AT+QMMSEDIT=0\r\n");  
	M3G_SendATCheck(1000,"AT+QFDEL=\"RAM:*\"\r\n" ); 
	err=M3G_SendATCheck(4000,"AT+QIDEACT=1\r\n"); 
	M3G_SendATCheck(1000,"AT+CGATT=1\r\n");  
	M3G_SendATCheck(2000,"AT+QICSGP=1,1,\"%s\",\"%s\",\"%s\",1\r\n",mms_configpt->apn,mms_configpt->user_name,mms_configpt->passwd);  
	M3G_SendATCheck(4000,"AT+QIACT=1\r\n"); 
	M3G_SendATCheck(1000,"AT+QMMSCFG=\"contextid\",1\r\n");	 
	M3G_SendATCheck(2000,"AT+QMMSCFG=\"mmsc\",\"%s\"\r\n",mms_configpt->url_mmsc); 
	M3G_SendATCheck(2000,"AT+QMMSCFG=\"proxy\",\"%s\",%s\r\n",mms_configpt->gateway,mms_configpt->port); 
	M3G_SendATCheck(1000,"AT+QMMSCFG=\"sendparam\",6,2,0,0,2,4\r\n");
	M3G_DataGetIPAddr();  
	return err;
}

CRED_Errors_t M3G_MMSDisConfig()
{
	CRED_Errors_t err = CRED_NO_ERROR; 
	M3G_SendATCheck(1000,"AT+QMMSEDIT=0\r\n");  
	M3G_SendATCheck(1000,"AT+QFDEL=\"RAM:*\"\r\n" ); 
	err=M3G_SendATCheck(4000,"AT+QIDEACT=1\r\n"); 
	return err;
}

CRED_Errors_t M3G_Send_MMS(struct mms_config*mms_configpt)
{
	size_t file_size;
	char file_buffer[640*480*2]; 
	CRED_Errors_t err = CRED_NO_ERROR;
	
	if(mms_configpt == NULL)
	{
		M3G_TRACE_ERROR("Null pointer\n");
		return CRED_ERROR_BAD_PARAMETER;
	}    
	if(strlen(contextid1_ipaddress)==0)
	{
		M3G_TRACE_ERROR("you must first config mms\n");
		return CRED_ERROR_UNKNOWN;
	} 
	M3G_SendATCheck(1000,"AT+QMMSEDIT=0\r\n");  
	M3G_SendATCheck(1000,"AT+QFDEL=\"RAM:*\"\r\n" );
	M3G_SendATCheck(1000,"AT+QMMSEDIT=1,1,\"%s\"\r\n",mms_configpt->address_to);  
	M3G_SendATCheck(1000,"AT+QMMSCFG=\"character\",\"ASCII\"\r\n");   
	M3G_SendATCheck(1000,"AT+QMMSEDIT=4,1,\"%s\"\r\n",mms_configpt->title);     
	memset(file_buffer,0x00,sizeof(file_buffer));
	M3G_ReadFile(mms_configpt->file_path,&file_size,file_buffer);
	M3G_AppendFileToRam(mms_configpt->file_path,file_size,file_buffer,1);
	M3G_SendATCheck(1000,"AT+QMMSEDIT=5,1,\"RAM:%s\"\r\n",mms_configpt->file_path); 
	memset(file_buffer,0x00,sizeof(file_buffer));
	M3G_ReadFile(mms_configpt->img_path,&file_size,file_buffer);
	M3G_AppendFileToRam(mms_configpt->img_path,file_size,file_buffer,1);
	M3G_SendATCheck(1000,"AT+QMMSEDIT=5,1,\"RAM:%s\"\r\n",mms_configpt->img_path); 
	err=M3G_SendATCheck(1000,"AT+QMMSEND=100\r\n");  
	M3G_SendATCheck(1000,"AT+QMMSCFG=\"character\",\"UTF8\"\r\n");  
	
	return err;
}
	
CRED_Errors_t M3G_GetHTTP(http_config*http_configpt)
{
	CRED_Errors_t err = CRED_NO_ERROR; 
	char rx_msg[200000];  
	char* rx_msg_endof_data;  
	char* rx_msg_startof_data;   
	read_size_global=0;
	unsigned int 	char_counter=0;  
	struct 			pollfd uart_fds;
	unsigned int 	read_size=0; 
	int pollres; 
	FILE *tcp_data_stream=NULL;
	
	if(http_configpt == NULL)
	{
		M3G_TRACE_ERROR("Null pointer\n");
		return CRED_ERROR_BAD_PARAMETER;
	}   
	if(strlen(contextid3_ipaddress)==0)
	{
		M3G_TRACE("you must first connect to data network\n");
		return err;
	} 
	M3G_TRACE("Entring url address\n");
	connect_state_global=0;
	M3G_SendAT(1000,"AT+QHTTPURL=%d,80\r\n",strlen(http_configpt->http_url));  
	while(connect_state_global==0)
	{
		usleep(1000);
	} 
	pthread_mutex_lock(&m3g_uartrx_mutex_global); 
	UART_Write(uart_fd_global,http_configpt->http_url,strlen(http_configpt->http_url));
	usleep(1000);  	 
	pthread_mutex_unlock(&m3g_uartrx_mutex_global); 
	
	M3G_TRACE("Sending Get\n");
	http_get_state_global=0;
	M3G_SendATCheck(1000,"AT+QHTTPGET=80\r\n");  
	M3G_SendATCheck(1000,"AT+QHTTPGET=80\r\n");  
	M3G_TRACE("waiting until get +QHTTPGET:\n"); 
	while(http_get_state_global==0)
	{
		usleep(1000);
	}  
	if(http_get_state_global<0)
	{
		M3G_TRACE_ERROR("error while getting headers\n");
		return CRED_ERROR_UNKNOWN; 
	}
	M3G_TRACE("Starting Reading Get\n"); 
	memset(rx_msg,0x00,sizeof(rx_msg));
	char_counter=0;
	uart_fds.events = POLLIN;
	uart_fds.fd=uart_fd_global;
	pthread_mutex_lock(&m3g_uartrx_mutex_global);
	UART_Write(uart_fd_global,"AT+QHTTPREAD=80\r\n", strlen("AT+QHTTPREAD=80\r\n")); 
	while(1)
	{ 
		pollres=poll(&uart_fds,1, 10000);  	
		if( pollres == 0)
		{
			M3G_TRACE("Http get Timout\n"); 
			pthread_mutex_unlock(&m3g_uartrx_mutex_global);  
			return err;
		}
		else if( pollres < 0)
		{
			M3G_TRACE_ERROR("http:error occur when retrieving response\n"); 
			pthread_mutex_unlock(&m3g_uartrx_mutex_global);  
			return CRED_ERROR_UNKNOWN;
		}
		UART_Read(uart_fd_global,rx_msg+char_counter,1500,&read_size); 
		//M3G_TRACE("Reading %d Bytes total %d Bytes\n",read_size,char_counter);
		if(rx_msg_endof_data=strstr(rx_msg+char_counter ,"+QHTTPREAD:")) break; 
		char_counter+=read_size;   
	}
	char_counter+=read_size;
	pthread_mutex_unlock(&m3g_uartrx_mutex_global);  
	rx_msg_startof_data=strstr(rx_msg,"CONNECT");
	if(rx_msg_endof_data==NULL)
	{
		M3G_TRACE_ERROR("cannot find CONNECT\n");
		return CRED_ERROR_UNKNOWN;
	}
	rx_msg_startof_data=strchr(rx_msg_startof_data,'\n');
	rx_msg_startof_data+=1;  
	tcp_data_stream=fopen (http_configpt->http_file_name, "w");
	if(tcp_data_stream==NULL)
	{
		M3G_TRACE_ERROR("cannot open file\n");
		return CRED_ERROR_UNKNOWN;
	} 
	M3G_TRACE("writing data to file %d Bytes\n",strlen(rx_msg_startof_data));
	fwrite(rx_msg_startof_data , 1 ,strlen(rx_msg_startof_data)-21, tcp_data_stream );
	fflush(tcp_data_stream);
	fclose(tcp_data_stream);
	strncpy(rx_msg_startof_data,http_configpt->http_fet_data_buff,strlen(rx_msg_startof_data)-21);
	return err; 
}

CRED_Errors_t M3G_aux_interface_init()
{
	CRED_Errors_t err = CRED_NO_ERROR;
 
	err=M3G_SendATCheck(1000,AT_QIDEACT_1); 
	if (err != CRED_NO_ERROR)
	{
		M3G_TRACE_ERROR("GSM Response ERROR %d \n",err);
		return err;
	} 
	err=M3G_SendATCheck(1000, AT_QDAC_2); 
	if (err != CRED_NO_ERROR)
	{
		M3G_TRACE_ERROR("GSM Response ERROR %d \n",err);
		return err;
	} 
	err=M3G_SendATCheck(1000, AT_QAUDMOD_1); 
	if (err != CRED_NO_ERROR)
	{
		M3G_TRACE_ERROR("GSM Response ERROR %d \n",err);
		return err;
	} 
	err=M3G_SendATCheck(1000, AT_QMIC_0_15);
	if (err != CRED_NO_ERROR)
	{
		M3G_TRACE_ERROR("GSM Response ERROR %d \n",err);
		return err;
	}
	err=M3G_SendATCheck(1000, AT_QSIDET_450);
	if (err != CRED_NO_ERROR)
	{
		M3G_TRACE_ERROR("GSM Response ERROR %d \n",err);
		return err;
	}
	err=M3G_SendATCheck(1000, AT_CLVL_60);
	if (err != CRED_NO_ERROR)
	{
		M3G_TRACE_ERROR("GSM Response ERROR %d \n",err);
		return err;
	}
	err=M3G_SendATCheck(1000, AT_QIIC_AUX_01);
	if (err != CRED_NO_ERROR)
	{
		M3G_TRACE_ERROR("GSM Response ERROR %d \n",err);
		return err;
	}
	err=M3G_SendATCheck(1000, AT_QIIC_AUX_02);
	if (err != CRED_NO_ERROR)
	{
		M3G_TRACE_ERROR("GSM Response ERROR %d \n",err);
		return err;
	}
	err=M3G_SendATCheck(1000, AT_QIIC_AUX_2C);
	if (err != CRED_NO_ERROR)
	{
		M3G_TRACE_ERROR("GSM Response ERROR %d \n",err);
		return err;
	}
	err=M3G_SendATCheck(1000, AT_QIIC_AUX_2D);
	if (err != CRED_NO_ERROR)
	{
		M3G_TRACE_ERROR("GSM Response ERROR %d \n",err);
		return err;
	}
	err=M3G_SendATCheck(1000, AT_QIIC_AUX_2F);
	if (err != CRED_NO_ERROR)
	{
		M3G_TRACE_ERROR("GSM Response ERROR %d \n",err);
		return err;
	}
	err=M3G_SendATCheck(1000, AT_QIIC_AUX_06);
	if (err != CRED_NO_ERROR)
	{
		M3G_TRACE_ERROR("GSM Response ERROR %d \n",err);
		return err;
	}
	err=M3G_SendATCheck(1000, AT_QIIC_AUX_24);
	if (err != CRED_NO_ERROR)
	{
		M3G_TRACE_ERROR("GSM Response ERROR %d \n",err);
		return err;
	}
	err=M3G_SendATCheck(1000, AT_QIIC_AUX_25);
	if (err != CRED_NO_ERROR)
	{
		M3G_TRACE_ERROR("GSM Response ERROR %d \n",err);
		return err;
	}
	err=M3G_SendATCheck(1000, AT_QIIC_AUX_26);
	if (err != CRED_NO_ERROR)
	{
		M3G_TRACE_ERROR("GSM Response ERROR %d \n",err);
		return err;
	}
	err=M3G_SendATCheck(1000, AT_QIIC_AUX_27);
	if (err != CRED_NO_ERROR)
	{
		M3G_TRACE_ERROR("GSM Response ERROR %d \n",err);
		return err;
	}
	err=M3G_SendATCheck(1000, AT_QIIC_AUX_31);
	if (err != CRED_NO_ERROR)
	{
		M3G_TRACE_ERROR("GSM Response ERROR %d \n",err);
		return err;
	}
	err=M3G_SendATCheck(1000, AT_QIIC_AUX_36);
	if (err != CRED_NO_ERROR)
	{
		M3G_TRACE_ERROR("GSM Response ERROR %d \n",err);
		return err;
	}
	return err;
}

CRED_Errors_t M3G_MailConfig(mail_config*email_config)
{
	CRED_Errors_t err = CRED_NO_ERROR;  
	
	if(email_config == NULL)
	{
		M3G_TRACE_ERROR("Null pointer\n");
		return CRED_ERROR_BAD_PARAMETER;
	} 
	if(strlen(contextid3_ipaddress)==0)
	{
		M3G_TRACE_ERROR("you must first connect to data network\n");
		return err;
	}
	M3G_SendATCheck(4000,"AT+QSMTPCFG=\"smtpserver\",\"%s\",%s\r\n", email_config->srvaddr, email_config->srvport); 
	M3G_SendATCheck(2000,"AT+QSMTPCFG=\"account\",\"%s\",\"%s\"\r\n",email_config->mail_user_name,email_config->mail_passwd); 
	M3G_SendATCheck(1000,"AT+QSMTPCFG=\"sender\",\"%s\",\"%s\"\r\n",email_config->sender_name,email_config->mail_user_name);
	return err;
}
	
CRED_Errors_t M3G_Send_Mail(mail_config*email_config)
{
	CRED_Errors_t err = CRED_NO_ERROR;  
	size_t file_size;
	char file_buffer[640*480*2]; 
	
	if(email_config == NULL)
	{
		M3G_TRACE_ERROR("Null pointer\n");
		return CRED_ERROR_BAD_PARAMETER;
	}   
	if(strlen(contextid3_ipaddress)==0)
	{
		M3G_TRACE_ERROR("you must first connect to data network\n");
		return CRED_ERROR_UNKNOWN;
	} 
	M3G_SendATCheck(1000,"AT+QSMTPCLR\r\n");  
	M3G_SendATCheck(1000,"AT+QFDEL=\"RAM:*\"\r\n");
	M3G_SendATCheck(1000,"AT+QSMTPDST=1,1,\"%s\"\r\n",email_config->address_to);    
	M3G_SendATCheck(1000,"AT+QSMTPSUB=0,\"%s\"\r\n",email_config->subject);
	
	//uploading body text
	connect_state_global=0;
	M3G_SendAT(1000,"AT+QSMTPBODY=1,%d,300\r\n",strlen(email_config->body_text));   
	while(connect_state_global==0)
	{
		usleep(1000);
	}
	M3G_TRACE("body_text uploading \n");
	pthread_mutex_lock(&m3g_uartrx_mutex_global); 
	qsmpt_state_global=0;
	UART_Write(uart_fd_global,email_config->body_text,strlen(email_config->body_text));    	 
	pthread_mutex_unlock(&m3g_uartrx_mutex_global); 
	while(qsmpt_state_global==0)
	{
		usleep(1000);
	}
	//uploading text file 
	memset(file_buffer,0x00,sizeof(file_buffer));
	file_size=0;
	M3G_ReadFile(email_config->file_path,&file_size,file_buffer);
	if(file_size!=0)
	{
		M3G_AppendFileToRam(email_config->file_path,file_size,file_buffer,1); 
		M3G_SendATCheck(1000,"AT+QSMTPATT=1,%d,\"RAM:%s\"\r\n",1,email_config->file_path); 
	}
	//uploading media file 
	file_size=0;
	memset(file_buffer,0x00,sizeof(file_buffer));
	M3G_ReadFile(email_config->img_path,&file_size,file_buffer);
	if(file_size!=0)
	{
		M3G_AppendFileToRam(email_config->img_path,file_size,file_buffer,2);  
		M3G_SendATCheck(1000,"AT+QSMTPATT=1,%d,\"RAM:%s\"\r\n",2,email_config->img_path); 
	}
	M3G_SendATCheck(1000,"AT+QSMTPPUT=300\r\n");  
	return CRED_NO_ERROR; 
}
     
CRED_Errors_t M3G_ReadFile(char*file_name,size_t* file_size,char* file_buffer)
{  
	FILE*fp_smtp;  
	
	if(file_name == NULL ||file_size == NULL ||file_buffer == NULL )
	{
		M3G_TRACE_ERROR("Null pointer\n");
		return CRED_ERROR_BAD_PARAMETER;
	} 
	fp_smtp=fopen(file_name,"rb"); 
	if(!fp_smtp)
	{
		M3G_TRACE_ERROR("cannot find %s\n",file_name);
		return CRED_ERROR_UNKNOWN;
	} 
	*file_size=0;
	memset(file_buffer,0x00,sizeof(file_buffer));   
	fseek(fp_smtp, 0, SEEK_END);   
	*file_size=ftell(fp_smtp);   
	fseek(fp_smtp, 0, SEEK_SET);
	fread(file_buffer,*file_size, 1, fp_smtp);
	fclose(fp_smtp);    
	return CRED_NO_ERROR;
}	

CRED_Errors_t M3G_AppendFileToRam(char*file_name,size_t file_size,char* file_buffer,int file_idx)
{  	
	if(file_name == NULL || file_buffer==NULL)
	{
		M3G_TRACE_ERROR("Null pointer\n");
		return CRED_ERROR_BAD_PARAMETER;
	} 
	connect_state_global=0;
	M3G_SendAT(1000,"AT+QFUPL=\"RAM:%s\",%d,200,1\r\n",file_name,file_size);   
	while(connect_state_global==0)
	{
		usleep(1000);
	}
	M3G_TRACE("file uploading \n");
	pthread_mutex_lock(&m3g_uartrx_mutex_global); 
	fileupload_state_global=0;
	UART_Write(uart_fd_global,file_buffer,file_size);    	 
	pthread_mutex_unlock(&m3g_uartrx_mutex_global); 
	while(fileupload_state_global==0)
	{
		usleep(1000);
	}  
	return CRED_NO_ERROR;
}

CRED_Errors_t M3G_check_status()
{
	CRED_Errors_t err = CRED_NO_ERROR;
	unsigned int timeoutcounter=5;
	while(1)
	{
		err =M3G_SendATCheck(400,"AT\r\n");
		if(CRED_NO_ERROR==err) return err;
		timeoutcounter--;
		if(timeoutcounter==0) break;
		sleep(1);
	} 
	return err;
}

CRED_Errors_t M3G_DTMFInit( )
{
	CRED_Errors_t err=CRED_NO_ERROR;
	int fd_flags;
	
	M3G_SendATCheck(1000,"AT+QTDMOD=1,0\r\n");
	err=M3G_SendATCheck(1000,"AT+QTONEDET =1\r\n");
	M3G_SendATCheck(1000,"AT+QTONEDET=4,1,0,3,100\r\n"); 
	
	if (pipe (dtmf_fd) < 0)
	{
	   M3G_TRACE_ERROR("dtmf pipe creation  error \n ");
	   return CRED_ERROR_UNKNOWN;
	}
	
    fd_flags = fcntl(dtmf_fd[0], F_GETFL);
    if (fd_flags < 0) 
    { 
		return err;
    }
    fcntl(dtmf_fd[0], F_SETFL, fd_flags | O_NONBLOCK);
	return err;
}

CRED_Errors_t M3G_DisableDTMF()
{
	CRED_Errors_t err=CRED_NO_ERROR; 
	err=M3G_SendATCheck(1000,"AT+QTONEDET =0\r\n");  
	return err;
}

CRED_Errors_t M3G_DetectModuleBaudeRate(unsigned int*speed)
{	
	unsigned int timeoutcounter;
	
	if(speed == NULL )
	{
		M3G_TRACE_ERROR("Null pointer\n");
		return CRED_ERROR_BAD_PARAMETER;
	}  
	UART_SetBaudrate(uart_fd_global,B115200); 
	sleep(1);
	timeoutcounter=4;
	while(timeoutcounter)
	{
		M3G_SendATCheck(100,"AT\r\n");
		if(strstr(m3g_rx_msg_global,"OK"))
		{
			M3G_TRACE("115200 Baude is detected\n");
			*speed=115200;
			return CRED_NO_ERROR;
		}
		timeoutcounter--; 
	} 
	UART_SetBaudrate(uart_fd_global,B921600); 
	sleep(1);
	timeoutcounter=4;
	while(timeoutcounter)
	{
		M3G_SendATCheck(100,"AT\r\n");
		if(strstr(m3g_rx_msg_global,"OK"))
		{
			M3G_TRACE("921600 Baude is detected\n"); 
			sleep(1);
			*speed=921600;
			return CRED_NO_ERROR;
		}
		timeoutcounter--; 
	}  
	M3G_TRACE("No Baude is detected\n");
	return CRED_NO_ERROR;
}

CRED_Errors_t M3G_ChangeBaudeRate(unsigned int speed)
{	
	unsigned int current_speed;
	
	M3G_DetectModuleBaudeRate(&current_speed);
	if(speed ==115200)
	{
		M3G_TRACE("current baude=%d changing to 115200\n",current_speed);
		M3G_SendATCheck(100,"AT+IPR=115200;&W\r\n");
		sleep(1);
		UART_SetBaudrate(uart_fd_global,B115200);
		sleep(1);
		M3G_DetectModuleBaudeRate(&current_speed);
		M3G_SendATCheck(100,"AT+IPR?\r\n");
		M3G_TRACE("m3g_rx_msg_global:/*%s*/\n",m3g_rx_msg_global);
	}
	else if(speed ==921600)
	{
		M3G_TRACE("current baude=%d changing to 921600\n",current_speed);
		M3G_SendATCheck(100,"AT+IPR=921600;&W\r\n");
		sleep(1);
		UART_SetBaudrate(uart_fd_global,B921600);
		sleep(1);
		M3G_DetectModuleBaudeRate(&current_speed);
		M3G_SendATCheck(100,"AT+IPR?\r\n");
		M3G_TRACE("m3g_rx_msg_global:/*%s*/\n",m3g_rx_msg_global);
	}
	return  CRED_NO_ERROR;
}

CRED_Errors_t m3g_init()
{
	CRED_Errors_t err = CRED_NO_ERROR; 
	enum m3g_pin_state pin_state;
	Signal_Quality Sig_Quality;
	Registration_Info Reg_Info;
	Module_Info  Mod_Info;
	Sim_Card_Info  Sim_Info;
	bool  Init_Status=false;
	bool Network_RegStatus=false;
	bool sim_status=false;
	unsigned int timout_counter;
	unsigned int speed;
	
	M3G_TRACE("Starting init....\n");
	
	if(CRED_NO_ERROR!=PCA9535_Init())
	{
		M3G_TRACE("fail to establish i2c communication with PCA9535 \n");
		err =  CRED_ERROR_UNKNOWN;
	}  
	if(CRED_NO_ERROR!=PCA9535_SetPinValue(PCA9535_IO0,IO0_3G_PWRENABLE,0))
	{
		M3G_TRACE_ERROR("fail to set 3G Module power enable\n");
		err =  CRED_ERROR_UNKNOWN;
	} 
	if(CRED_NO_ERROR!=PCA9535_SetPinValue(PCA9535_IO0,IO0_3G_PWRKEY,0))
	{
		M3G_TRACE_ERROR("fail to set 3G Module power key\n");
		err =  CRED_ERROR_UNKNOWN;
	}
	sleep(1);
	if(CRED_NO_ERROR!=PCA9535_SetPinValue(PCA9535_IO0,IO0_3G_PWRKEY,1))
	{
		M3G_TRACE_ERROR("fail to set 3G Module power key\n");
		err =  CRED_ERROR_UNKNOWN;
	}
	sleep(6); 
	pthread_mutex_init(&m3g_uartrx_mutex_global, NULL); 
	sem_init(&m3g_parsing_sem, 0, 0);

	if (CRED_NO_ERROR != UART_Init(&uart_fd_global,CRED_UART_TTYS0))
	{
		M3G_TRACE("fail to init m3g\n");
		err =  CRED_ERROR_UNKNOWN;
	}
	pthread_create(&m3g_uartrx_pthread_global ,NULL,M3G_UARTPoll,NULL);
	pthread_create(&m3g_parse_pthread_global ,NULL,M3G_RxParseThread,NULL);
	usleep(10000);
	err=M3G_check_status();
	if(CRED_NO_ERROR != err) 
	{
		M3G_TRACE_ERROR("M3G device is not ready...\n");
		return err;
	}
	M3G_SendATCheck(1000,"AT+QSIMDET=1,0\r\n");  
	M3G_SendATCheck(1000,"AT+CRC=1\r\n");  
	M3G_SendATCheck(1000,"AT+QSIMSTAT=1\r\n");
	M3G_SendATCheck(1000,"ATV1\r\n"); 
	M3G_SendATCheck(1000,"ATE1\r\n"); 
	M3G_SendATCheck(1000,"AT+CMEE=2\r\n"); 
	
	err=M3G_check_sim_status(&sim_status);
	if(sim_status==false)
	{
		M3G_TRACE_ERROR("SIM is not ready1...\n");
		return err;
	}
	err=M3G_GetPinState(&pin_state);
	if(pin_state!=SIM_PIN_PUK_OK)
	{
		M3G_TRACE_ERROR("PIN is not ready...\n");
		return err;
	}
	timout_counter=5;
	while(1)
	{
		err=M3G_Check_MODULE_Init_Status(&Init_Status);
		timout_counter--;
		if(Init_Status==true)
		{
			M3G_TRACE("Module Init OK...\n");
			break;
		}
		if(timout_counter)
		{
			M3G_TRACE("Module is not yet ready...\n");
			sleep(2);
			continue;
		}
		else
		{
			M3G_TRACE_ERROR("Module is not ready2...\n");
			return err;
		}
	}
	timout_counter=10;
	Network_RegStatus=false;
	while(Network_RegStatus==false)
	{
		Network_RegStatus=false;
		M3G_Sim_Network_Registration_Status(&Network_RegStatus);
		if(timout_counter<1)
		{
			M3G_TRACE_ERROR("module is not registred to network timout...\n");
			return err;
		}
		sleep(2);
		timout_counter--;
	}
	M3G_Signal_Quality(&Sig_Quality);
	M3G_Registration_Information(&Reg_Info); 
	M3G_Module_Information(&Mod_Info);
	M3G_Sim_Card_Information(&Sim_Info);
	M3G_Init_SMS( );
	M3G_DTMFInit();
	M3G_aux_interface_init();
	return err;
}

CRED_Errors_t m3g_Term()
{
	CRED_Errors_t err = CRED_NO_ERROR;
	M3G_TRACE("Terminating m3g\n");
	m3g_uartpoll_run=0;
	sleep(1);
	pthread_cancel(m3g_uartrx_pthread_global);
	usleep(1000);
	pthread_mutex_destroy(&m3g_uartrx_mutex_global);
	usleep(1000);
	UART_Term(uart_fd_global);
	uart_fd_global=-1;
	return err;
}

CRED_Errors_t m3g_sim_ready(bool *sim_status)
{
	CRED_Errors_t err = CRED_NO_ERROR;
	int i=0;
	
	if(sim_status == NULL )
	{
		M3G_TRACE_ERROR("Null pointer\n");
		return CRED_ERROR_BAD_PARAMETER;
	}  
	i=0;
	err = M3G_check_sim_status(uart_fd_global);
	while((CRED_NO_ERROR != err) && (i<10))
	{ 
		M3G_TRACE("SIM Not ready\n");
		i++;
		usleep(10000);	
		err = M3G_check_sim_status(sim_status);
	}
	return err;
}

CRED_Errors_t M3G_SMSDelete(uint8_t sms_idx)
{   
	CRED_Errors_t err=CRED_NO_ERROR; 
	M3G_TRACE("M3G_SMSDelete\n");
	err=M3G_SendATCheck(2000,"AT+CMGD=%d,0\r\n",sms_idx);
	return err;   
}
 
CRED_Errors_t M3G_CallInProceessing(char * call_rx_msg,char*phone_number)
{
	CRED_Errors_t err = CRED_NO_ERROR; 
	char*rx_msg_pt;
	char*rx_msg_pt1; 
	
	if(call_rx_msg==NULL)
	{
		M3G_TRACE_ERROR("Null pointer \n");
		return CRED_ERROR_BAD_PARAMETER; 
	} 
	rx_msg_pt=strstr(call_rx_msg,"RING");
	if(rx_msg_pt==NULL)
	{
		M3G_TRACE_ERROR("cannot find RING\n"); 
		return CRED_ERROR_UNKNOWN;
	} 
	M3G_SendATCheck(1000,"AT+CLCC\r\n"); 
	rx_msg_pt=strstr(m3g_rx_msg_global,"+CLCC:");
	if(rx_msg_pt==NULL)
	{
		M3G_TRACE_ERROR("cannot find +CLCC:\n"); 
		return CRED_ERROR_UNKNOWN;
	}  
	rx_msg_pt=strchr(rx_msg_pt,'"');   
	if(rx_msg_pt==NULL)
	{
		M3G_TRACE_ERROR("cannot find \"\n"); 
		return CRED_ERROR_UNKNOWN;
	} 
	rx_msg_pt++;
	rx_msg_pt1= strchr(rx_msg_pt,'"'); 
	if(rx_msg_pt1==NULL)
	{
		M3G_TRACE_ERROR("cannot find \"\n"); 
		return CRED_ERROR_UNKNOWN;
	}
	memset(phone_number,0x00,sizeof(phone_number));
	rx_msg_pt1[0]='\0';
	strncpy(phone_number,rx_msg_pt,rx_msg_pt1-rx_msg_pt);
	M3G_TRACE("phone_number:%s\n",phone_number);
	
	struct Ns_event_Str TsEvent;
	struct Telephony_event_Str m3g_event_Str;
	m3g_event_Str.event_detail=(uint8_t*)phone_number;
	m3g_event_Str.telephony_action=CRED_TS_INCOMMING_CALL;
	TsEvent.As_event=CRED_TS_CALL;
	TsEvent.event_detail=(uint8_t*)&m3g_event_Str;
	if(M3G_TS_Notify ) M3G_TS_Notify(TsEvent); 
	
	return err;   
}

CRED_Errors_t M3G_SMSRead(unsigned int sms_idx,struct sms_config*received_sms_info )
{
	CRED_Errors_t err = CRED_NO_ERROR;
	unsigned int numbers[5];
	unsigned int sizenum;
	char*rx_msg_pt;
	char*rx_msg_pt1;
	char ira[400];
	char rx_msg[1000];
	
	if(received_sms_info ==NULL)
	{
		M3G_TRACE_ERROR("Null pointer \n");
		return CRED_ERROR_BAD_PARAMETER; 
	} 
	memset(rx_msg,0x00,sizeof(rx_msg));
	M3G_SelectCharEncod(M3G_CHAR_UCS2);
	M3G_SendATCheck(1000,"AT+CMGF=1\r\n");    
	err=M3G_SendAT(1000,"AT+CMGR=%d\r\n",sms_idx);
	pthread_mutex_lock (&m3g_uartrx_mutex_global);
	err=M3G_RxReadUntilGetStr(rx_msg,"OK");
	pthread_mutex_unlock (&m3g_uartrx_mutex_global);
	rx_msg_pt=strstr(rx_msg,"+CMGR:");
	if(rx_msg_pt==NULL)
	{
		M3G_TRACE_ERROR("cannot find +CMGR:\n"); 
		return err;
	}  
	memset(received_sms_info,0x00,sizeof(struct sms_config)); 
	
	received_sms_info->index=sms_idx;
	M3G_TRACE("received_sms_info->index :%d\n",received_sms_info->index);
	rx_msg_pt=strchr(rx_msg_pt,','); 
	if(rx_msg_pt==NULL)
	{
		M3G_TRACE_ERROR("Null pointer line:%d\n",__LINE__);
		return CRED_ERROR_UNKNOWN;
	} 
	rx_msg_pt=strchr(rx_msg_pt,'"');  
	if(rx_msg_pt==NULL)
	{
		M3G_TRACE_ERROR("Null pointer line:%d\n",__LINE__);
		return CRED_ERROR_UNKNOWN;
	}
	rx_msg_pt++;
	rx_msg_pt1= strchr(rx_msg_pt,'"'); 
	if(rx_msg_pt1==NULL)
	{
		M3G_TRACE_ERROR("Null pointer line:%d\n",__LINE__);
		return CRED_ERROR_UNKNOWN;
	}
	memset(ira,0x00,sizeof(ira));
	strncpy(ira,rx_msg_pt,rx_msg_pt1-rx_msg_pt);
	M3G_CharSet_UTF8(ira,received_sms_info->num);
	M3G_TRACE("received_sms_info->num :%s\n",received_sms_info->num);
	
	rx_msg_pt=strchr(rx_msg_pt1,',');  
	if(rx_msg_pt==NULL)
	{
		M3G_TRACE_ERROR("Null pointer line:%d\n",__LINE__);
		return CRED_ERROR_UNKNOWN;
	}
	rx_msg_pt++;
	rx_msg_pt=strchr(rx_msg_pt,',');  
	if(rx_msg_pt==NULL)
	{
		M3G_TRACE_ERROR("Null pointer line:%d\n",__LINE__);
		return CRED_ERROR_UNKNOWN;
	}
	rx_msg_pt++;
	rx_msg_pt=strchr(rx_msg_pt,'"');  
	if(rx_msg_pt==NULL)
	{
		M3G_TRACE_ERROR("Null pointer line:%d\n",__LINE__);
		return CRED_ERROR_UNKNOWN;
	}
	rx_msg_pt++;
	rx_msg_pt1= strchr(rx_msg_pt,'"'); 
	if(rx_msg_pt1==NULL)
	{
		M3G_TRACE_ERROR("Null pointer line:%d\n",__LINE__);
		return CRED_ERROR_UNKNOWN;
	}
	strncpy(received_sms_info->date,rx_msg_pt,rx_msg_pt1-rx_msg_pt);
	M3G_TRACE("received_sms_info->date :%s\n",received_sms_info->date);
	
	rx_msg_pt=strchr(rx_msg_pt1,'\n');  
	if(rx_msg_pt==NULL)
	{
		M3G_TRACE_ERROR("Null pointer line:%d\n",__LINE__);
		return CRED_ERROR_UNKNOWN;
	}
	rx_msg_pt++; 
	rx_msg_pt1= strchr(rx_msg_pt,'\n'); 
	if(rx_msg_pt1==NULL)
	{
		M3G_TRACE_ERROR("Null pointer line:%d\n",__LINE__);
		return CRED_ERROR_UNKNOWN;
	}
	memset(ira,0x00,sizeof(ira));
	strncpy(ira,rx_msg_pt,rx_msg_pt1-rx_msg_pt-1);
	M3G_CharSet_UTF8(ira,received_sms_info->msg);
	M3G_TRACE("received_sms_info->msg :%s\n",received_sms_info->msg); 
 
	return err;   
}

CRED_Errors_t M3G_SMSInProceessing(char * sms_rx_msg,struct sms_config*received_sms_info )
{
	CRED_Errors_t err = CRED_NO_ERROR;
	unsigned int numbers[5];
	unsigned int sizenum;
	char*rx_msg_pt;
	
	if(sms_rx_msg==NULL || received_sms_info ==NULL)
	{
		M3G_TRACE_ERROR("Null pointer \n");
		return CRED_ERROR_BAD_PARAMETER; 
	} 
	rx_msg_pt=strstr(sms_rx_msg,"+CMTI:");
	if(rx_msg_pt==NULL)
	{
		M3G_TRACE_ERROR("cannot find +CMTI:\n"); 
		return CRED_ERROR_UNKNOWN;
	}
	M3G_ExtractIntegers(rx_msg_pt,numbers,&sizenum) ; 
	M3G_SMSRead(numbers[0],received_sms_info );
	
	struct Ns_event_Str TsEvent;
	struct Telephony_event_Str m3g_event_Str; 
	m3g_event_Str.event_detail=(uint8_t*)received_sms_info;
	m3g_event_Str.telephony_action=CRED_TS_INCOMMING;
	TsEvent.As_event=CRED_TS_SMS;
	TsEvent.event_detail=(uint8_t*)&m3g_event_Str; 
	if(M3G_TS_Notify) M3G_TS_Notify(TsEvent);
	
	return err;   
}
CRED_Errors_t M3G_SMSOutProceessing(char * sms_rx_msg,unsigned int*sms_idx)
{  
	CRED_Errors_t err = CRED_NO_ERROR;
	unsigned int numbers[5];
	unsigned int sizenum; 
	char*rx_msg_pt;
	
	if(sms_rx_msg==NULL || sms_idx==NULL)
	{
		M3G_TRACE_ERROR("Null pointer \n");
		return CRED_ERROR_BAD_PARAMETER; 
	} 
	rx_msg_pt=strstr(sms_rx_msg,"+CMGS:");
	if(rx_msg_pt==NULL)
	{
		M3G_TRACE_ERROR("cannot find +CMGS:\n"); 
		return CRED_ERROR_BAD_PARAMETER;
	}
	M3G_ExtractIntegers(rx_msg_pt,numbers,&sizenum) ;  
	*sms_idx=numbers[0];
	M3G_TRACE("msg was sent with index %d\n",*sms_idx);   
	
	struct Ns_event_Str TsEvent;
	struct Telephony_event_Str m3g_event_Str;
	m3g_event_Str.event_detail=(uint8_t*)sms_idx;
	m3g_event_Str.telephony_action=CRED_TS_OUTGOING;
	TsEvent.As_event=CRED_TS_SMS;
	TsEvent.event_detail=(uint8_t*)&m3g_event_Str; 
	if(M3G_TS_Notify) M3G_TS_Notify(TsEvent);  
	
	return err;   
}

CRED_Errors_t M3G_GetDTMFChar(int* dtmf_char, int* dtmf_size )
{	
	CRED_Errors_t err = CRED_NO_ERROR;  
	unsigned int dtmfint; 
	
	if(dtmf_char==NULL || dtmf_size==NULL)
	{
		M3G_TRACE_ERROR("Null pointer \n");
		return CRED_ERROR_BAD_PARAMETER; 
	}
	*dtmf_size=read(dtmf_fd[0], &dtmfint, sizeof(int)); 
	if(*dtmf_size==-1)
	{
		/*M3G_TRACE("No dtmf at the moment\n");*/
		*dtmf_char=DSIL;
		return err;
	}
	switch(dtmfint)
    {
        case 35 :
            *dtmf_char=DPND;
        break;
        case 42 :
            *dtmf_char=DSTAR;
        break;
        case 48 :
            *dtmf_char=D0;
        break;
        case 49 :
            *dtmf_char=D1;
        break;
        case 50 :
            *dtmf_char=D2;
        break;
        case 51 :
            *dtmf_char=D3;
        break;
        case 52 :
            *dtmf_char=D4;
        break;
        case 53 :
            *dtmf_char=D5;
        break;
        case 54 :
            *dtmf_char=D6;
        break;
        case 55 :
            *dtmf_char=D7;
        break;
        case 56 :
            *dtmf_char=D8;
        break;
        case 57 :
            *dtmf_char=D9;
        break;
    }
	return err;
}

CRED_Errors_t M3G_DTMFInProceessing(char * dtmf_rx_msg)
{  
	CRED_Errors_t err=CRED_NO_ERROR;   
	char*dtmchr;	
	unsigned int numbers[5];
	unsigned int sizenum;  
	
	if(dtmf_rx_msg==NULL)
	{
		M3G_TRACE_ERROR("Null pointer \n");
		return CRED_ERROR_BAD_PARAMETER; 
	} 
	dtmchr=strstr(dtmf_rx_msg ,"+QTONEDET:");	
	if(dtmchr == NULL)
	{
		M3G_TRACE_ERROR("cannot find +QTONEDET:\n"); 
		return CRED_ERROR_BAD_PARAMETER;
	}
	M3G_ExtractIntegers(dtmchr,numbers,&sizenum) ;   
	M3G_TRACE("DTMF=%c\n",numbers[0]); 
	write(dtmf_fd[1], &numbers[0], sizeof(int));

	struct Ns_event_Str TsEvent;
	struct Telephony_event_Str m3g_event_Str;
	m3g_event_Str.event_detail=(uint8_t*)&numbers[0];
	m3g_event_Str.telephony_action=CRED_TS_INCOMMING;
	TsEvent.As_event=CRED_TS_DTMF;
	TsEvent.event_detail=(uint8_t*)&m3g_event_Str; 
	if(M3G_TS_Notify) M3G_TS_Notify(TsEvent);    
	usleep(10000);
	
	return err;   
}

CRED_Errors_t M3G_TCPInProceessing(char * tcp_rx_msg)
{  	
	CRED_Errors_t err=CRED_NO_ERROR;   
	char*dtmchr;	
	unsigned int numbers[5];
	unsigned int sizenum;  
	char*rx_msg_pt;
	char*rx_msg_pt1;
	
	if(tcp_rx_msg==NULL)
	{
		M3G_TRACE_ERROR("Null pointer \n");
		return CRED_ERROR_BAD_PARAMETER; 
	} 
	M3G_TRACE("incomming tcp packet\n");
	M3G_ExtractIntegers(tcp_rx_msg,numbers,&sizenum); 
	if(sizenum>1)
	{
		M3G_TRACE("Receive data in direct push access mode connect id=%d data_size=%d\n",numbers[0],numbers[1]);
		rx_msg_pt=strchr(tcp_rx_msg,'\n'); 
		rx_msg_pt++;
		rx_msg_pt1=strchr(rx_msg_pt,'\n'); 
		memset(m3g_tcp_in_msg_global,0x00,sizeof(m3g_tcp_in_msg_global));
		strncpy(m3g_tcp_in_msg_global, rx_msg_pt,rx_msg_pt1-rx_msg_pt-1); 
		if(numbers[0]<12) m3g_tcp_packet_is_got_global[numbers[0]]=1;
		else
		{
			 M3G_TRACE_ERROR("connect id out the range");
			 return CRED_ERROR_UNKNOWN;
		}
		M3G_TRACE("tcp_msg :%s\n",m3g_tcp_in_msg_global);
	} 
	return err; 
	  
}
CRED_Errors_t M3G_COPSProceessing(char * rx_msg_pt)
{  	 
	CRED_Errors_t err = CRED_NO_ERROR;
	struct 			pollfd uart_fds;
	unsigned int 	read_size=0;  
	char rx_msg_temp[500]; 
	char* rx_msg_pt_temp; 
	char* rx_msg_pt1_temp; 
	int pollres;
	
	if(rx_msg_pt==NULL)
	{
		M3G_TRACE_ERROR("Null pointer \n");
		return CRED_ERROR_BAD_PARAMETER; 
	} 
	strcpy(rx_msg_temp,rx_msg_pt);     
	if(strstr(rx_msg_temp,"ERROR")) 
	{
		M3G_TRACE_ERROR("ERROR rx_msg_temp=%s",rx_msg_temp);
		return CRED_ERROR_UNKNOWN;
	} 
	else if(strstr(rx_msg_temp,"OK")) 
	{
		goto COPSPROC;
	} 
	uart_fds.events = POLLIN;
	uart_fds.fd=uart_fd_global;
	pthread_mutex_lock(&m3g_uartrx_mutex_global); 
	rx_msg_pt_temp=rx_msg_temp+strlen(rx_msg_temp);
	while(1)
	{   
		pollres=poll(&uart_fds,1,1000);  
		if( pollres == 0)
		{
			M3G_TRACE_ERROR("M3G_COPSProceessing Timout\n"); 
			pthread_mutex_unlock(&m3g_uartrx_mutex_global);
			return CRED_ERROR_UNKNOWN;
		}
		else if( pollres < 0)
		{
			M3G_TRACE_ERROR("M3G_COPSProceessing error occur when retrieving response\n"); 
			pthread_mutex_unlock(&m3g_uartrx_mutex_global);
			return CRED_ERROR_UNKNOWN;
		}
		UART_Read(uart_fd_global,rx_msg_pt_temp,200,&read_size);
		if(strstr(rx_msg_temp,"OK") || strstr(rx_msg_temp,"ERROR")) 
		{
			//M3G_TRACE("breaking with rx_msg_temp=%s",rx_msg_temp);
			break;
		}  
		rx_msg_pt_temp +=read_size; 
	}
	pthread_mutex_unlock(&m3g_uartrx_mutex_global);
	if(strstr(rx_msg_temp,"ERROR")) 
	{
		M3G_TRACE_ERROR("ERROR rx_msg_temp=%s",rx_msg_temp);
		return CRED_ERROR_UNKNOWN;
	} 
	COPSPROC:
	rx_msg_pt_temp=rx_msg_temp;
	while(1)
	{
		char opsname[20];
		rx_msg_pt_temp=strchr(rx_msg_pt_temp,'(');
		if(rx_msg_pt_temp== NULL) return err;
		rx_msg_pt_temp=strchr(rx_msg_pt_temp,'"');
		if(rx_msg_pt_temp== NULL) return err;
		rx_msg_pt_temp++;
		rx_msg_pt1_temp=strchr(rx_msg_pt_temp,'"');
		if(rx_msg_pt1_temp== NULL) return err;
		memset(opsname,0x00,sizeof(opsname));
		strncpy(opsname,rx_msg_pt_temp,rx_msg_pt1_temp-rx_msg_pt_temp);
		M3G_TRACE("opsname=%s\n",opsname);
	}
	COPSEND:
	return err; 
}
char m3g_dummy_char;
CRED_Errors_t M3G_EmailOutProceessing(char * rx_msg_pt)
{  	
	CRED_Errors_t err=CRED_NO_ERROR;   
	
	if(rx_msg_pt==NULL)
	{
		M3G_TRACE_ERROR("Null pointer \n");
		return CRED_ERROR_BAD_PARAMETER; 
	} 
	M3G_SendATCheck(1000,"AT+QSMTPCLR\r\n");
	M3G_SendATCheck(1000,"AT+QFDEL=\"RAM:*\"\r\n");
	if(strstr(rx_msg_pt,"+QSMTPPUT: 0,0"))
	{
		M3G_TRACE("mail send OK\n");  
		m3g_dummy_char=1;
	}
	else
	{
		M3G_TRACE("mail send FAIL\n"); 
		m3g_dummy_char=0;
	}
	
	struct Ns_event_Str TsEvent;
	struct Telephony_event_Str m3g_event_Str;
	m3g_event_Str.event_detail=(uint8_t*)&m3g_dummy_char;
	m3g_event_Str.telephony_action=CRED_TS_OUTGOING;
	TsEvent.As_event=CRED_TS_EMAIL;
	TsEvent.event_detail=(uint8_t*)&m3g_event_Str; 
	if(M3G_TS_Notify) M3G_TS_Notify(TsEvent);  
	
	return err;  
}
CRED_Errors_t M3G_MMSOutProceessing(char * rx_msg_pt)
{  	
	CRED_Errors_t err=CRED_NO_ERROR;
	unsigned int numbers[5];
	unsigned int sizenum; 
	
	if(rx_msg_pt==NULL)
	{
		M3G_TRACE_ERROR("Null pointer \n");
		return CRED_ERROR_BAD_PARAMETER; 
	} 
	M3G_ExtractIntegers(rx_msg_pt,numbers,&sizenum) ; 
	if( numbers[0]==0)
	{
		M3G_TRACE("mms send OK\n");
		m3g_dummy_char=1;
	} 
	else
	{
		M3G_TRACE("mms send fail\n");
		m3g_dummy_char=0;
	}
	M3G_SendATCheck(1000,"AT+QMMSEDIT=0\r\n");  
	err=M3G_SendATCheck(1000,"AT+QFDEL=\"RAM:*\"\r\n" );
	
	struct Ns_event_Str TsEvent;
	struct Telephony_event_Str m3g_event_Str;
	m3g_event_Str.event_detail=(uint8_t*)&m3g_dummy_char;
	m3g_event_Str.telephony_action=CRED_TS_OUTGOING;
	TsEvent.As_event=CRED_TS_MMS;
	TsEvent.event_detail=(uint8_t*)&m3g_event_Str; 
	if(M3G_TS_Notify) M3G_TS_Notify(TsEvent);  
		
	return err;  
}
CRED_Errors_t M3G_EventsProcessing(char*rx_msg)
{  
	char *rx_msg_pt;
	CRED_Errors_t err = CRED_NO_ERROR;
	
	if(rx_msg == NULL)
	{
		M3G_TRACE_ERROR("NULL pointer\n");
		return CRED_ERROR_BAD_PARAMETER;
	}
	//M3G_TRACE("Parsed:/*%s*/\n",rx_msg);
	  
	if(rx_msg_pt=strstr(rx_msg ,"+COPS:"))
	{  
		M3G_COPSProceessing(rx_msg_pt);
		return err;
	} 
	else if(rx_msg_pt=strstr(rx_msg ,"CONNECT"))
	{   
		connect_state_global=1;
		return err;
	} 
	else if(rx_msg_pt=strstr(rx_msg ,"+CSQ:")) 
	{  
			return err;
	}
	else if(rx_msg_pt=strstr(rx_msg ,"+QIOPEN:")) 
	{	
		unsigned int numbers[5];
		unsigned int sizenum; 
		M3G_ExtractIntegers(rx_msg_pt,numbers,&sizenum) ; 
		if(numbers[1]==0)
		{
			M3G_TRACE("TCP Socket open with connect id :%d is OK\n",numbers[0]);
			m3g_tcp_connections_status_global[numbers[0]]=1;
		}
		else
		{
			M3G_TRACE("TCP Socket open with connect ip :%d failed and error code =%d\n",numbers[0],numbers[1]);
			m3g_tcp_connections_status_global[numbers[0]]=numbers[1];
		} 
		return err;
	}
	else if(rx_msg_pt=strstr(rx_msg ,"+QIURC: \"closed\"")) 
	{  
			return err;
	}
	else if(rx_msg_pt=strstr(rx_msg ,"+QIURC:  \"pdpdeact\"")) 
	{  
			M3G_DataGetIPAddr( );
			return err;
	}
	else if(rx_msg_pt=strstr(rx_msg ,"+QSMTPBODY")) 
	{  
		strcpy(qsmpt_rx_global,rx_msg_pt);
		qsmpt_state_global=1;
		return err; 
	}  
	else if(rx_msg_pt=strstr(rx_msg ,"+QIURC: \"incoming full\"")) 
	{  
		M3G_TRACE_ERROR("incoming connection reaches the limit\n");
		return err;
	}
	else if(rx_msg_pt=strstr(rx_msg ,"+QIURC: \"recv\"")) 
	{ 
		M3G_TCPInProceessing(rx_msg_pt);
		return err;
	}
	else if(rx_msg_pt=strstr(rx_msg ,"+QTONEDET:"))
	{  
		M3G_DTMFInProceessing(rx_msg_pt);
		return err;
	} 
	else if(rx_msg_pt=strstr(rx_msg ,"+CRING: VOICE"))
	{  
		char phone_number[20];
		M3G_CallInProceessing(rx_msg_pt,phone_number);
		return err;
	} 
	else if(rx_msg_pt=strstr(rx_msg ,"RING"))
	{  
		char phone_number[20];
		M3G_CallInProceessing(rx_msg_pt,phone_number);
		return err;
	}  
	else if(rx_msg_pt=strstr(rx_msg ,"+CMTI: \"SM\""))
	{ 
		struct sms_config received_sms_info;
		M3G_SMSInProceessing(rx_msg_pt,&received_sms_info);  
		return err;
	}  
	else if(rx_msg_pt=strstr(rx_msg ,"NO CARRIER"))
	{  
		M3G_TRACE("call is ended with NO CARRIER\n");
		Dial_end_state_global=NO_CARRIER;
		return err;
	}  
	else if(rx_msg_pt=strstr(rx_msg ,"NO ANSWER"))
	{  
		M3G_TRACE("call is ended with NO ANSWER\n");
		Dial_end_state_global=NO_ANSWER;
		return err;
	} 
	else if(rx_msg_pt=strstr(rx_msg ,"BUSY"))
	{  
		M3G_TRACE("call is ended with BUSY\n");
		Dial_end_state_global=BUSY;
		return err;
	}  
	else if(rx_msg_pt=strstr(rx_msg ,"NO DIALTONE"))
	{  
		M3G_TRACE("call is ended with NO DIALTONE\n");
		Dial_end_state_global=NO_DIALTONE;
		return err;
	}  
	else if(rx_msg_pt=strstr(rx_msg ,"+CPIN: READY"))
	{
		M3G_TRACE("PIN is Ready ...\n");  
		return err;
	} 
	else if(rx_msg_pt=strstr(rx_msg ,"+CPIN: NOT READY"))
	{  
		M3G_TRACE("PIN is Not  Ready ...\n");  
		return err;
	} 
	else if(rx_msg_pt=strstr(rx_msg ,"+QIND: SMS DONE"))
	{  
			return err;
	} 
	else if(rx_msg_pt=strstr(rx_msg ,"+QIND: PB DONE"))
	{  
			return err;
	} 
	else if(rx_msg_pt=strstr(rx_msg ,"+QSIMSTAT:"))
	{  
		if(strstr(rx_msg_pt,"+QSIMSTAT: 1,0"))
		{
			M3G_TRACE("SIM is removed\n");
		}
		else if(strstr(rx_msg_pt,"+QSIMSTAT: 1,1"))
		{
			M3G_TRACE("SIM is inserted\n");
		}
		return err;
	} 
	else if(rx_msg_pt=strstr(rx_msg ,"+QUSIM:"))
	{  
			return err;
	} 
	else if(rx_msg_pt=strstr(rx_msg ,"+QSMTPPUT:"))
	{  
		M3G_EmailOutProceessing(rx_msg_pt);
		return err;
	}
	else if(rx_msg_pt=strstr(rx_msg ,"+QMMSEND:"))
	{
		M3G_MMSOutProceessing(rx_msg_pt);
		return err;
	}
	else if(rx_msg_pt=strstr(rx_msg ,"+QHTTPGET:")) 
	{
		M3G_TRACE("getting +QHTTPGET:\n");
		if(strstr(rx_msg_pt ,"+QHTTPGET: 0"))
		{
			M3G_TRACE("http get OK\n");
			http_get_state_global=1; 
		}
		else
		{
			M3G_TRACE("http get Fail\n");
			http_get_state_global=-1;
		}	 
		return err;
	} 
	else if(rx_msg_pt=strstr(rx_msg ,"SEND FAIL")) 
	{
		tcp_send_state_global=-1; 
		return err;
	}
	else if(rx_msg_pt=strstr(rx_msg ,"SEND OK")) 
	{
		tcp_send_state_global=1;
		return err;
	} 
	else if(rx_msg_pt=strstr(rx_msg ,"+CMGS:")) 
	{
		unsigned int sms_idx;
		M3G_SMSOutProceessing(rx_msg_pt,&sms_idx);
		return err;
	}
	else if(rx_msg_pt=strstr(rx_msg ,"+QFUPL:")) 
	{
		fileupload_state_global=1;
		return err;
	}
	else
	{
		//M3G_TRACE("others:%s",rx_msg);  
		return err;
	}
	 
}

CRED_Errors_t M3G_SendUSSD(char*ussd_msg,char*ussd_resp)
{  	
	CRED_Errors_t err=CRED_NO_ERROR; 
	char* rx_msg_pt;
	char* rx_msg_pt1;
	char rx_msg[1000];
	char ussd_msg_ucs2[1000];
	char ussd_resp_ucs2[1000];
	
	if(ussd_msg==NULL || ussd_resp==NULL)
	{
		M3G_TRACE_ERROR("null pointer\n");
		return CRED_ERROR_BAD_PARAMETER;
	} 
	M3G_SelectCharEncod(M3G_CHAR_UCS2);
	memset(ussd_msg_ucs2,0x00,sizeof(ussd_msg_ucs2));
	M3G_UTF8_CharSet(ussd_msg,ussd_msg_ucs2); 
	M3G_SendATCheck(10000,"AT+CUSD=1,\"%s\",15\r\n",ussd_msg_ucs2);
	rx_msg_pt=strstr(m3g_rx_msg_global,"+CUSD:");
	if(rx_msg_pt==NULL)
	{
		M3G_TRACE_ERROR("cannot find +CUSD:\n");
		return CRED_ERROR_UNKNOWN;
	}
	rx_msg_pt=strchr(rx_msg_pt,'"');
	if(rx_msg_pt==NULL)
	{
		M3G_TRACE_ERROR("cannot handle a null pointer\n");
		return CRED_ERROR_UNKNOWN;
	}
	rx_msg_pt++;
	rx_msg_pt1=strchr(rx_msg_pt,'"');
	if(rx_msg_pt1==NULL)
	{
		M3G_TRACE_ERROR("cannot handle a null pointer\n");
		return CRED_ERROR_UNKNOWN;
	}
	memset(ussd_resp_ucs2,0x00,sizeof(ussd_resp_ucs2));
	strncpy(ussd_resp_ucs2,rx_msg_pt,rx_msg_pt1-rx_msg_pt);
	M3G_CharSet_UTF8(ussd_resp_ucs2,ussd_resp);
	return err; 
} 	 
CRED_Errors_t M3G_CancelUSSDSession( )
{  	
	CRED_Errors_t err=CRED_NO_ERROR; 
	err=M3G_SendATCheck(1000,"AT+CUSD=2\r\n");
	return err; 
} 	 

CRED_Errors_t M3G_GetTCPInMsg(struct Tcp_config *tcp_config)
{  	
	CRED_Errors_t err=CRED_NO_ERROR; 
	unsigned int tim_out_counter ; 
	
	if(tcp_config==NULL)
	{
		M3G_TRACE_ERROR("Null pointer\n"); 
		return CRED_ERROR_BAD_PARAMETER;	
	} 
	M3G_TRACE("Getting response tcp packet\n");
	tim_out_counter=tcp_config->tim_out_ms ;
	while(m3g_tcp_packet_is_got_global[tcp_config->connect_id]==0)
	{
		if(tim_out_counter<1)
		{
			M3G_TRACE_ERROR("Timout when getting tcp packet\n");
			return CRED_ERROR_UNKNOWN;
		}
		usleep(1000);
		tim_out_counter--;
	}
	strcpy(tcp_config->tcp_resp_msg,m3g_tcp_in_msg_global);
	M3G_TRACE("tcp packet is got:%s\n",tcp_config->tcp_resp_msg);
	return err; 
} 	 

CRED_Errors_t M3G_Send_TCP(struct Tcp_config *tcp_config)
{ 
	CRED_Errors_t err = CRED_NO_ERROR;
	char rx_msg[2000];
	char* rx_msg_pt;
	char* rx_msg_pt1;
	unsigned int numbers[5];
	unsigned int sizenum; 
	unsigned int timout_counter;
	
	if(strlen(contextid3_ipaddress)==0)
	{
		M3G_TRACE_ERROR("you must first connect to data network\n");
		return CRED_ERROR_UNKNOWN;
	}
	if(tcp_config==NULL)
	{
		M3G_TRACE_ERROR("Null pointer \n");
		return CRED_ERROR_BAD_PARAMETER;
	}

	memset(rx_msg,0x00,sizeof(rx_msg));
	pthread_mutex_lock (&m3g_uartrx_mutex_global);
	err=M3G_SendATNoCheck(4000,"AT+QIOPEN=3,%d,\"TCP\",\"%s\",%s,0,1\r\n",tcp_config->connect_id,tcp_config->IP_Addr ,tcp_config->port_server);
	err=M3G_RxReadUntilGetStr(rx_msg,"+QIOPEN:");
	pthread_mutex_unlock (&m3g_uartrx_mutex_global);
	rx_msg_pt=strstr(rx_msg ,"+QIOPEN:");
	if(rx_msg_pt==NULL || err != CRED_NO_ERROR)
	{
		M3G_TRACE_ERROR("cannot find +QIOPEN: \n");
		if(tcp_config->tcp_end)
		{
			*tcp_config->tcp_end = TCP_END_TIMEOUT;
		}
		return CRED_ERROR_UNKNOWN;
	}
	M3G_ExtractIntegers(rx_msg_pt,numbers,&sizenum) ; 
	if(numbers[1]==0)
	{
		M3G_TRACE("TCP Socket open with connect id :%d is OK\n",numbers[0]);
	}
	else
	{
		M3G_TRACE("TCP Socket open with connect ip :%d failed and error code =%d\n",numbers[0],numbers[1]);
		if(tcp_config->tcp_end)
		{
			*tcp_config->tcp_end = TCP_END_FAIL;
		}
		return CRED_ERROR_UNKNOWN;
	} 

	M3G_SendATCheck(4000,"AT+QISEND=%d,%d\r\n",tcp_config->connect_id,strlen(tcp_config->tcp_msg));
	memset(rx_msg,0x00,sizeof(rx_msg));
	pthread_mutex_lock(&m3g_uartrx_mutex_global); 
	UART_Write(uart_fd_global,  tcp_config->tcp_msg,strlen(  tcp_config->tcp_msg)); 
	err=M3G_RxReadUntilGetStr(rx_msg,"SEND");
	pthread_mutex_unlock(&m3g_uartrx_mutex_global); 
	rx_msg_pt=strstr(rx_msg ,"SEND OK");
	if(rx_msg_pt==NULL || err != CRED_NO_ERROR)
	{
		M3G_TRACE_ERROR("cannot find SEND OK\n");
		return CRED_ERROR_UNKNOWN;
	}
	if(tcp_config->tcp_resp_msg==NULL)
	{
		M3G_TRACE_ERROR("response buffer is a null pointer\n");
		return CRED_ERROR_UNKNOWN;	
	} 
	memset(rx_msg,0x00,sizeof(rx_msg));
	pthread_mutex_lock(&m3g_uartrx_mutex_global);  
	err=M3G_RxReadUntilGetStr(rx_msg,"+QIURC:");
	pthread_mutex_unlock(&m3g_uartrx_mutex_global); 
	if(err!=CRED_NO_ERROR)
	{
		M3G_TRACE_ERROR("timeout +QIURC \n");
		if(tcp_config->tcp_end)
		{
			*tcp_config->tcp_end = TCP_END_TIMEOUT;
		}
		return err;
	}
	rx_msg_pt=strstr(rx_msg ,"+QIURC: \"recv\"");
	if(rx_msg_pt==NULL)
	{
		M3G_TRACE_ERROR("tcp connection is closed by remote\n");
		if(tcp_config->tcp_end)
		{
			*tcp_config->tcp_end = TCP_END_CLOSED;
		}
		return CRED_ERROR_UNKNOWN;
	}
	
	if(tcp_config->tcp_end)
	{
		*tcp_config->tcp_end = TCP_END_FAIL;
	}
	
	M3G_TRACE("rx_msg_pt=%s\n",rx_msg_pt);
	rx_msg_pt=strstr(rx_msg_pt,"\r\n");
	if(rx_msg_pt==NULL)
	{
		M3G_TRACE_ERROR("cannot find carriage return\n");
		return CRED_ERROR_UNKNOWN;
	}
	rx_msg_pt+=2;
	rx_msg_pt=strstr(rx_msg_pt,"\r\n");
	if(rx_msg_pt==NULL)
	{ 
		pthread_mutex_lock(&m3g_uartrx_mutex_global);  
		if(strlen(rx_msg)<1500)
		{
			err=M3G_RxReadUntilGetStr(rx_msg+strlen(rx_msg),"\r\n");
		}
		pthread_mutex_unlock(&m3g_uartrx_mutex_global); 
	}
	rx_msg_pt=strstr(rx_msg ,"+QIURC:");
	if(rx_msg_pt==NULL)
	{
		M3G_TRACE_ERROR("error cannot handle a null pointer\n");
		return CRED_ERROR_UNKNOWN;	
	}
	rx_msg_pt=strchr(rx_msg_pt,'\n');  
	if(rx_msg_pt==NULL)
	{
		M3G_TRACE_ERROR("error cannot handle a null pointer\n");
		return CRED_ERROR_UNKNOWN;	
	}
	rx_msg_pt+=2;
	rx_msg_pt1=rx_msg+strlen(rx_msg);  
	if((rx_msg_pt1-rx_msg_pt-1<0) || (rx_msg_pt1-rx_msg_pt-1>1500))
	{
		M3G_TRACE_ERROR("length error \n");
		return CRED_ERROR_UNKNOWN;
	}
	strncpy(tcp_config->tcp_resp_msg, rx_msg_pt,rx_msg_pt1-rx_msg_pt-1);
	if(tcp_config->tcp_end)
	{
		*tcp_config->tcp_end = TCP_END_OK;
	}
	//M3G_TRACE("tcp_config->tcp_resp_msg=%s\n",tcp_config->tcp_resp_msg); 
	return CRED_NO_ERROR;
}

CRED_Errors_t M3G_Send_UDP(struct Tcp_config *udp_config)
{ 
	CRED_Errors_t err = CRED_NO_ERROR;
	char rx_msg[100];
	char* rx_msg_pt;
	char* rx_msg_pt1;
	unsigned int numbers[5];
	unsigned int sizenum;  
	static unsigned int timout_counter;

	if(strlen(contextid3_ipaddress)==0)
	{
		M3G_TRACE_ERROR("you must first connect to data network\n");
		return CRED_ERROR_UNKNOWN;
	}
	if(udp_config==NULL)
	{
		M3G_TRACE_ERROR("Null pointer \n");
		return CRED_ERROR_BAD_PARAMETER;
	}
	M3G_TRACE("open udp socket\n");
	m3g_tcp_connections_status_global[udp_config->connect_id]=0;
	err=M3G_SendAT(4000,"AT+QIOPEN=3,%d,\"UDP SERVICE\",\"127.0.0.1\",0,3030,0\r\n",
						udp_config->connect_id);  
	timout_counter=1000;			 
	while(m3g_tcp_connections_status_global[udp_config->connect_id]==0 )
	{ 
		timout_counter--;
		if(timout_counter<1) break;
		usleep(1000);
	}
	M3G_TRACE("checking opening udp result\n");
	if(m3g_tcp_connections_status_global[udp_config->connect_id]!=1)
	{
		M3G_TRACE_ERROR("error occur while udp open\n");
		return CRED_ERROR_UNKNOWN;
	}
	err=M3G_SendATCheck(4000,"AT+QISEND=%d,10,\"%s\",%s\r\n",udp_config->connect_id,udp_config->IP_Addr,udp_config->port_server);
	M3G_TRACE("writing msg to uart\n");
	pthread_mutex_lock(&m3g_uartrx_mutex_global); 
	tcp_send_state_global=0;
	UART_Write(uart_fd_global,udp_config->tcp_msg,strlen(udp_config->tcp_msg));   
	pthread_mutex_unlock(&m3g_uartrx_mutex_global); 
	M3G_TRACE("sending data\n");  
	timout_counter=1000;	
	while(tcp_send_state_global == 0)
	{
		timout_counter--;
		if(timout_counter<1) break;
		usleep(1000);
	}
	if(tcp_send_state_global == 1)
	{
		M3G_TRACE("UDP Send OK\n"); 
	}
	else
	{
		M3G_TRACE_ERROR("UDP Send Fail\n");
		return err;
	}
	usleep(1000);
	err=M3G_SendATCheck(1000,"AT+QISEND=%d,0\r\n",udp_config->connect_id); 
	rx_msg_pt= strchr(m3g_rx_msg_global,'=');
	if(rx_msg_pt==NULL) 
	{
		M3G_TRACE_ERROR("cannot find =\n" );
		return CRED_ERROR_UNKNOWN;
	}
	rx_msg_pt=strstr(rx_msg_pt,"+QISEND:");
	if(rx_msg_pt==NULL) 
	{
		M3G_TRACE_ERROR("cannot find +QISEND: %s,%d\n",m3g_rx_msg_global,err);
		return CRED_ERROR_UNKNOWN;
	}
	M3G_ExtractIntegers(rx_msg_pt,numbers,&sizenum) ;
	M3G_TRACE("data size sent :%d\n",numbers[0]);
	return CRED_NO_ERROR;
}

CRED_Errors_t M3G_TCPClose(uint8_t connect_id)
{
	CRED_Errors_t err = CRED_NO_ERROR;  
	err=M3G_SendATCheck(2000,"AT+QICLOSE=%d\r\n",connect_id);  
	return err;
} 

CRED_Errors_t M3G_DataGetIPAddr()
{  
	CRED_Errors_t err = CRED_NO_ERROR; 
	char*rx_msg_pt;
	char*rx_msg_pt1;
	unsigned int ips_counter=0;  
	unsigned int connectid; 
	 
	M3G_SendATCheck(1000,"AT+QIACT?\r\n");   
	rx_msg_pt=m3g_rx_msg_global;   
	ips_counter=0;
	
	while(ips_counter<3)
	{  
		rx_msg_pt=strstr(rx_msg_pt,"+QIACT:");
		if(rx_msg_pt==NULL)
		{
			M3G_TRACE("cannot find +QIACT:\n");  
			return err; 
		}   
		while((rx_msg_pt[0]<48 || rx_msg_pt[0]>57) && rx_msg_pt[0]!='\0')  rx_msg_pt++;
		connectid=atoi(rx_msg_pt); 
		rx_msg_pt=strchr(rx_msg_pt,'"');
		rx_msg_pt++;
		rx_msg_pt1=strchr(rx_msg_pt,'"'); 
		if(connectid==1)
		{ 
			strncpy(contextid1_ipaddress,rx_msg_pt,rx_msg_pt1-rx_msg_pt);  
			M3G_TRACE("contextid1_ipaddress:%s\n",contextid1_ipaddress);  
		}
		else if(connectid==2)
		{ 
			strncpy(contextid2_ipaddress,rx_msg_pt,rx_msg_pt1-rx_msg_pt);  
			M3G_TRACE("contextid2_ipaddress:%s\n",contextid2_ipaddress);   
		}
		else if(connectid==3)
		{
			strncpy(contextid3_ipaddress,rx_msg_pt,rx_msg_pt1-rx_msg_pt);  
			M3G_TRACE("contextid3_ipaddress:%s\n",contextid3_ipaddress);   
		} 
		ips_counter++;
	} 
	M3G_TRACE("ips_counter ==%d\n",ips_counter);
	return err; 
}

CRED_Errors_t M3G_DataConfig(char*apn,char*user_name,char*passwd)
{
	CRED_Errors_t err = CRED_NO_ERROR;  

	if(apn==NULL || user_name == NULL || passwd == NULL)
	{
		M3G_TRACE_ERROR("NULL pointer\n");
		return CRED_ERROR_BAD_PARAMETER;
	} 
	M3G_SendATCheck(4000,"AT+QIDEACT=3\r\n"); 
	M3G_SendATCheck(1000,"AT+QICSGP=3,1,\"%s\",\"%s\",\"%s\",1\r\n",apn,user_name,passwd);    
	err=M3G_SendATCheck(6000,"AT+QIACT=3\r\n");     
	M3G_SendATCheck(1000,"AT+QHTTPCFG=\"contextid\",3\r\n"); 
	M3G_SendATCheck(1000,"AT+QSMTPCFG=\"contextid\",3\r\n"); 
	M3G_SendATCheck(1000,"AT+QSMTPCFG=\"ssltype\",1\r\n");  
	M3G_SendATCheck(1000,"AT+QSMTPCFG=\"sslctxid\",1\r\n");
	M3G_SendATCheck(1000,"AT+QSSLCFG=\"ciphersuite\",1,\"0xffff\"\r\n"); 
	M3G_SendATCheck(1000,"AT+QSSLCFG=\"seclevel\",1,2\r\n"); 
	M3G_SendATCheck(1000,"AT+QSSLCFG=\"sslversion\",1,1\r\n");    
	memset(contextid1_ipaddress,0x00,sizeof(contextid1_ipaddress));
	memset(contextid2_ipaddress,0x00,sizeof(contextid2_ipaddress));
	memset(contextid3_ipaddress,0x00,sizeof(contextid3_ipaddress)); 
	M3G_DataGetIPAddr( );  
	M3G_TRACE("contextid1_ipaddress :%s\n",contextid1_ipaddress);
	M3G_TRACE("contextid2_ipaddress :%s\n",contextid2_ipaddress);
	M3G_TRACE("contextid3_ipaddress :%s\n",contextid3_ipaddress);  
	return err;
}

CRED_Errors_t M3G_DataDeconfig( )
{
	CRED_Errors_t err = CRED_NO_ERROR;   
	err=M3G_SendATCheck(4000,"AT+QIDEACT=3\r\n");  
	return err;
}

CRED_Errors_t M3G_Get_IP_Addr(char* ip_addr)
{  
	if(ip_addr==NULL)
	{
		M3G_TRACE_ERROR("NULL pointer\n");
		return CRED_ERROR_BAD_PARAMETER;
	}  
	memset(contextid1_ipaddress,0x00,sizeof(contextid1_ipaddress));
	memset(contextid2_ipaddress,0x00,sizeof(contextid2_ipaddress));
	memset(contextid3_ipaddress,0x00,sizeof(contextid3_ipaddress)); 
	M3G_DataGetIPAddr( );  
	M3G_TRACE("contextid1_ipaddress :%s\n",contextid1_ipaddress);
	M3G_TRACE("contextid2_ipaddress :%s\n",contextid2_ipaddress);
	M3G_TRACE("contextid3_ipaddress :%s\n",contextid3_ipaddress);  
	strcpy(ip_addr,contextid3_ipaddress);
	return CRED_NO_ERROR;
}
CRED_Errors_t M3G_Connect_Data_Network(Tcp_config*tcp)
{ 
	if(tcp==NULL)
	{
		M3G_TRACE_ERROR("NULL pointer\n");
		return CRED_ERROR_BAD_PARAMETER;
	}  
	return M3G_DataConfig(tcp->apn,tcp->user_name,tcp->passwd);
}

CRED_Errors_t M3G_Disconnect_Data_Network()
{
	return M3G_DataDeconfig( );
}
CRED_Errors_t M3G_Connect_TCP_Server(Tcp_config*tcp_config)
{ 
	if(tcp_config==NULL)
	{
		M3G_TRACE_ERROR("NULL pointer\n");
		return CRED_ERROR_BAD_PARAMETER;
	} 
	if(strstr(tcp_config->connect_mode,"TCP"))
	{
		return M3G_Send_TCP(tcp_config);
	}
	else if(strstr(tcp_config->connect_mode,"UDP"))
	{
		return M3G_Send_UDP(tcp_config);
	}
}

CRED_Errors_t M3G_Disconnect_TCP_Server(uint8_t connect_id)
{	
	return M3G_TCPClose(connect_id);
}

CRED_Errors_t M3G_InitNotif(CRED_MW_Errors_t *m3g_ts_notify_ptr)
{	
	CRED_Errors_t err = CRED_NO_ERROR;  
	M3G_TRACE("adding notif to m3g\n");
	M3G_TS_Notify=m3g_ts_notify_ptr;
	return err;
}


