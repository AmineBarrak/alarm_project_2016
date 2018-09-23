/* alarm.c
 *
 *
 * Copyright (c)2015 comelit R&D Tunisia
 *
 */
#include "rf_crow_cred.h"
#include "notification_cred.h"
#include "alarm_cred.h"
#include "gpio_cred.h"
#include "power_cred.h"
#include "telephony_cred.h"
#include "rtc_cred.h"
alarm_service_rf_keypad_t *rf_keypad_global_ptr;
alarm_service_rf_remote_t *rf_remote_global_ptr;
alarm_service_rf_output_t *rf_output_global_ptr;
alarm_service_rf_zone_t *rf_zone_global_ptr;
alarm_service_user_code_t *user_code_global_ptr;
alarm_service_installer_code_t *installer_code_global_ptr;
alarm_service_manufacturer_code_t *manufacturer_global_ptr;
alarm_service_profile_t *profile_global_ptr;
alarm_service_receiver_t *receiver_global_ptr;
alarm_service_cctv_t *cctv_global_ptr;
alarm_service_area_t *area_global_ptr;
alarm_service_arm_option_t *arm_option_global_ptr;
alarm_service_arm_status_t *alarm_arm_status_global_ptr;
alarm_system_info_t *alarm_system_info_global_ptr;
static struct Ns_event_Str arm_event_to_alarm_service;		/*!< Notification Structure. */

alarm_service_user_code_t current_user_code_str;
alarm_service_installer_code_t current_installer_code_str;

RF_Device_Type_t rf_device_type_action; /*!< last Device type used for last action. */
static alarm_service_rf_last_action_t rf_last_action_str;
static struct Ns_Notify_Str alarm_service_notify_S;
static int as_notification_exit = 0;
static uint8_t as_alarm_notify = 0;
static uint8_t rf_module_jamming = 0x01;
static uint8_t module_Initialisation = 0x00;
uint8_t alarm_service_init_flag =0;
uint8_t arm_mode_global = CRED_ARM_MODE_DISARM;
uint8_t output_index_global = 0;
#ifdef CRED_ARC_DEFAULT
uint8_t arc_ip_global =1;
uint8_t arc_sms_global =1;
uint8_t arc_cid_voice_global =1;
#else
uint8_t arc_ip_global =0;
uint8_t arc_sms_global =0;
uint8_t arc_cid_voice_global =0;
#endif
//char arc_number_cid[20]="035360748";
//char arc_number_sms[20]="3389445254";
char arc_number_cid[20]="70834124";
char arc_number_sms[20]="98605812";
//char arc_ip_address[20]="172.20.6.202";
char arc_ip_address[20]="195.31.184.179";
int arc_ip_port = 36000;
int arc_ip_account = 10;
//int arc_cid_voice_account = 10;
int arc_cid_voice_account = 11;
int arc_sms_account = 10;
static uint8_t rf_devices_subevent[8]={EV_TYPE_NOT_USED,EV_TYPE_ALARM_ZONES,EV_TYPE_ALARM_ZONES,EV_TYPE_ALARM_ZONES,
				EV_TYPE_FAULT_ZONES,EV_TYPE_ALARM_ZONES,
				EV_TYPE_SABOTAGE_ZONES,EV_TYPE_FAULT_ZONES
};
static uint8_t rf_zone_alarm_status[8]={CRED_ZONE_ALARM_STATUS_NO_ALARM,
				CRED_ZONE_ALARM_STATUS_ALARM,CRED_ZONE_ALARM_STATUS_ALARM,
				CRED_ZONE_ALARM_STATUS_ALARM,CRED_ZONE_ALARM_STATUS_FAULT,
				CRED_ZONE_ALARM_STATUS_ALARM,CRED_ZONE_ALARM_STATUS_SABOTAGE,
				CRED_ZONE_ALARM_STATUS_FAULT
};
static uint8_t rf_output_subevent[8]={EV_TYPE_NOT_USED,EV_TYPE_NOT_USED,EV_TYPE_NOT_USED,EV_TYPE_NOT_USED,
				EV_TYPE_FAULT_OUTPUT,EV_TYPE_NOT_USED,
				EV_TYPE_SABOTAGE_OUTPUT,EV_TYPE_FAULT_OUTPUT
};
static uint8_t pir_detector_status[8]={EV_ALARM_ZONE_NOT_USED,EV_ALARM_ZONE_NOT_USED,EV_ALARM_ZONE_NOT_USED,EV_ALARM_ZONE_NOT_USED,
				EV_FAULT_BATTERY,EV_ALARM_RF_ZONE_PIR_MASKED,
				EV_SABOTAGE_TAMPER,EV_FAULT_TROUBLE
};
static uint8_t magnet_detector_status[8]={EV_ALARM_ZONE_NOT_USED,EV_ALARM_ZONE_NOT_USED,EV_ALARM_ZONE_NOT_USED,EV_ALARM_ZONE_NOT_USED,
				EV_FAULT_BATTERY,EV_ALARM_ZONE_NOT_USED,
				EV_SABOTAGE_TAMPER,EV_FAULT_TROUBLE
};
/*static uint8_t siren_output_status[8]={EV_ALARM_ZONE_NOT_USED,EV_ALARM_ZONE_NOT_USED,EV_ALARM_ZONE_NOT_USED,EV_ALARM_ZONE_NOT_USED,
				EV_FAULT_BATTERY,EV_ALARM_ZONE_NOT_USED,
				EV_SABOTAGE_TAMPER,EV_FAULT_TROUBLE
};*/
static uint8_t output_status[8]={EV_ALARM_ZONE_NOT_USED,EV_ALARM_ZONE_NOT_USED,EV_ALARM_ZONE_NOT_USED,EV_ALARM_ZONE_NOT_USED,
				EV_FAULT_BATTERY,EV_ALARM_ZONE_NOT_USED,
				EV_SABOTAGE_TAMPER,EV_FAULT_TROUBLE
};
/*static uint8_t pir_siren_status[8];={EV_ALARM_ZONE_NOT_USED,EV_ALARM_ZONE_NOT_USED,EV_ALARM_ZONE_NOT_USED,EV_ALARM_ZONE_NOT_USED,
				EV_FAULT_RF_SIREN_BATTERY,EV_ALARM_ZONE_NOT_USED,
				EV_SABOTAGE_RF_SIREN_TAMPER,EV_FAULT_RF_SIREN
};*/
static uint8_t panic_status[8]={EV_ALARM_ZONE_NOT_USED,EV_ALARM_ZONE_NOT_USED,EV_ALARM_ZONE_NOT_USED,EV_ALARM_ZONE_NOT_USED,
				EV_FAULT_BATTERY,EV_ALARM_ZONE_NOT_USED,
				EV_ALARM_ZONE_NOT_USED,EV_FAULT_TROUBLE
};
static uint8_t remote_status[8]={EV_ALARM_ZONE_NOT_USED,EV_ALARM_ZONE_NOT_USED,EV_ALARM_ZONE_NOT_USED,EV_ALARM_ZONE_NOT_USED,
				EV_FAULT_BATTERY,EV_ALARM_ZONE_NOT_USED,
				EV_ALARM_ZONE_NOT_USED,EV_FAULT_TROUBLE
};
static uint8_t area_index_tab[8]={CRED_RF_ZONE_AREA1,CRED_RF_ZONE_AREA2,CRED_RF_ZONE_AREA3,CRED_RF_ZONE_AREA4,
				CRED_RF_ZONE_AREA5,CRED_RF_ZONE_AREA6,
				CRED_RF_ZONE_AREA7,CRED_RF_ZONE_AREA8
};
pthread_t alarm_notify_pthread_t;
static struct Ns_Notify_Str *alarm_service_event_tmp;
CRED_MW_Errors_t get_alarm_notification(struct Ns_Notify_Str *Ps_Struct , struct Ns_event_Str *Ps_evt);
//static RF_Global_str_t *rf_detectors_ptr;
static CRED_AS_EVENT_PARAM_t *rf_zone_alarm_to_ns_event=NULL,*rf_zone_alarm_to_ns_Start=NULL;
static int rf_zone_to_ns_index=0;
void alarm_notify_thread(void *notify_S )
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	struct Ns_Notify_Str *alarm_notify_str =  (struct Ns_Notify_Str *)notify_S;
	int rval_ptr;
	/*Received Power service Events;*/
	struct Ns_event_Str alarm_service_event;
	//struct alarm_event_Str *alarm_event_ptr;
		/* Alarm RF Zone to NS events*/
	struct Ns_event_Str rf_zone_alarm_event;
	
	
	if (notify_S == NULL)
	{
		ALARM_TRACE_ERROR("Bad notify_S param NULL pointer \n");
		pthread_exit(&rval_ptr);
	}
	rf_zone_alarm_to_ns_Start = malloc(sizeof(CRED_AS_EVENT_PARAM_t) * MAX_EVENT_BUFFER_SIZE);
	
	if (rf_zone_alarm_to_ns_Start == NULL)
	{
		ALARM_TRACE_ERROR("Malloc failed: rf_zone_alarm_to_ns_Start NULL pointer \n");
		pthread_exit(&rval_ptr);
	}
	rf_zone_to_ns_index=0;
	rf_zone_alarm_to_ns_event = rf_zone_alarm_to_ns_Start;
	while (1)
	{
		sem_wait(&alarm_notify_str->As_sem);
		
		if (as_notification_exit)
		{
			ALARM_TRACE_ERROR("alarm_notify_thread EXIT !!!! \n");
			goto exit;
		}
		
		if (alarm_notify_str->As_length == 0)
		{
			continue;
		}
		
		get_alarm_notification(alarm_notify_str , (struct Ns_event_Str *)&(alarm_service_event.As_event));
		//alarm_event_ptr = (struct alarm_event_Str *)alarm_service_event.event_detail;
		ALARM_TRACE_INFO("alarm received events :%d !!!! \n",alarm_service_event.As_event);
		
		switch(alarm_service_event.As_event)
		{
			case CRED_ALARM_RF_MODULE:
			{
				ALARM_TRACE("CRED_ALARM_RF_MODULE\n");
				alarm_rf_module_event_Notify(alarm_service_event.event_detail);
			}
			break;
			case CRED_ALARM_RF_DETECTORS:
			{
				//rf_detectors_ptr = (struct RF_Global_str_t *)alarm_service_event.event_detail;
				ALARM_TRACE("CRED_ALARM_RF_DETECTORS\n");
				alarm_rf_detectors_event_Notify(alarm_service_event.event_detail);
			}
			break;
			case CRED_ALARM_RF_OUTPUT:
			{
				ALARM_TRACE("CRED_ALARM_RF_OUTPUT\n");
				alarm_rf_output_event_Notify(alarm_service_event.event_detail);
			}
			break;
			case CRED_ALARM_RF_PENDANT:
			{
				ALARM_TRACE("CRED_ALARM_RF_PENDANT\n");
				alarm_rf_pendant_event_Notify(alarm_service_event.event_detail);
			}
			break;
			case CRED_ALARM_RF_KEYPAD:
			{
				ALARM_TRACE("CRED_ALARM_RF_KEYPAD\n");
				alarm_rf_keypad_event_Notify(alarm_service_event.event_detail);
			}
			break;
			case CRED_ALARM_CONFIG_SR:
			{
				ALARM_TRACE("CRED_ALARM_CONFIG_SR\n");
			}
			break;
			case CRED_ALARM_RMOTE_MGT_SR:
			{
				ALARM_TRACE("CRED_ALARM_RMOTE_MGT_SR\n");
			}
			break;
			case CRED_ALARM_TOUCH:
			{
				ALARM_TRACE("CRED_ALARM_TOUCH\n");
			}
			break;
			case CRED_ALARM_KEYPAD:
			{
				ALARM_TRACE("CRED_ALARM_KEYPAD\n");
			}
			break;
			case CRED_ALARM_CONFIG_SERVICE:
			{
				ALARM_TRACE("CRED_ALARM_CONFIG_SERVICE\n");
			}
			break;
			case CRED_ALARM_CONFIG_TIME:
			{
				ALARM_TRACE("CRED_ALARM_CONFIG_TIME\n");
			}
			break;
			case CRED_ALARM_CONFIG_USERS_REMOTE:
			{
				ALARM_TRACE("CRED_ALARM_CONFIG_USERS_REMOTE\n");
			}
			break;
			case CRED_ALARM_CONFIG_ZONE:
			{
				ALARM_TRACE("CRED_ALARM_CONFIG_ZONE\n");
			}
			break;
			case CRED_ALARM_CONFIG_OUTPUTS:
			{
				ALARM_TRACE("CRED_ALARM_CONFIG_OUTPUTS\n");
			}
			break;
			case CRED_ALARM_CONFIG_AREAS:
			{
				ALARM_TRACE("CRED_ALARM_CONFIG_AREAS\n");
			}
			break;
			case CRED_ALARM_CONFIG_COMMUNICATION:
			{
				ALARM_TRACE("CRED_ALARM_CONFIG_COMMUNICATION\n");
			}
			break;
			case CRED_ALARM_CONFIG_SYSTEM_OPTION:
			{
				ALARM_TRACE("CRED_ALARM_CONFIG_SYSTEM_OPTION\n");
			}
			break;
			case CRED_ALARM_RF_ACTION:
			{
				ALARM_TRACE("CRED_ALARM_RF_ACTION\n");
				alarm_rf_action_event(alarm_service_event.event_detail);
			}
			break;
			case CRED_ALARM_ARM_PROCEDURE:
			{
				ALARM_TRACE("CRED_ALARM_ARM_PROCEDURE\n");
				alarm_arm_procedure((alarm_service_arm_status_t *)alarm_service_event.event_detail);
			}
			break;
			case CRED_ALARM_SMS_CONTROL:
			{
				ALARM_TRACE("CRED_ALARM_SMS_CONTROL\n");
				/*sms_control_str* sms_control_global_str = alarm_service_event.event_detail;
				ALARM_TRACE("sms idx  : %d\n",sms_control_global_str->sms_control_msg_idx);
				ALARM_TRACE("user code  : %s\n",sms_control_global_str->sms_control_user_code);
				ALARM_TRACE("sms from :%s\n",sms_control_global_str->sms_control_number);
				ALARM_TRACE("actions  :%d\n",(unsigned int)sms_control_global_str->sms_control_action);
				ALARM_TRACE("partition_mode  :%d\n",(unsigned int)sms_control_global_str->sms_control_partition);
				ALARM_TRACE("sms_control_attribute_number  :%d\n",sms_control_global_str->sms_control_attribute_number);*/
			}
			break;
			default:
				ALARM_TRACE("CRED AS unknow notification\n");
			break;
		}
		if(as_alarm_notify == 1){
				//Ps_to_AsEvent.As_event = CRED_AS_BATTERY;
				//power_to_ns_index++;
				//Power_to_Ns_event++;
				//if(power_to_ns_index > MAX_EVENT_BUFFER_SIZE){
				//	power_to_ns_index =0;
				//	Power_to_Ns_event = Power_to_Ns_Start;
				//}	
		}
	}

exit:
	free(rf_zone_alarm_to_ns_Start);
	pthread_exit(&rval_ptr);
}
 CRED_MW_Errors_t alarm_events_init(struct Ns_Notify_Str *as_event_S)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;

	if (as_event_S == NULL)
	{
		ALARM_TRACE_ERROR("Bad event_tmp param NULL pointer \n");
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	
	as_event_S->Ns_event = malloc(sizeof(struct Ns_event_Str) * AS_MAX_EVENT_BUFFER_SIZE);
	as_event_S->As_read = 0;
	as_event_S->As_write = 0;
	as_event_S->As_length = 0;
	sem_init(&as_event_S->As_sem, 0, 0);
	alarm_service_event_tmp = as_event_S;
	
	return err;

}

 CRED_MW_Errors_t alarm_events_term(struct Ns_Notify_Str *as_event_S)
 {
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;

	if (as_event_S == NULL)
	{
		ALARM_TRACE_ERROR("Bad ALARM alarm_service_event_tmp param NULL pointer \n");
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	
	//free(ps_event_S->Ns_event);
	sem_destroy(&as_event_S->As_sem);
	
	return err;
 }
CRED_MW_Errors_t get_alarm_notification(struct Ns_Notify_Str *alarm_service_struct , struct Ns_event_Str *alarm_service_evt)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	
	if (alarm_service_evt == NULL)
	{
		ALARM_TRACE_ERROR("Bad ALARM alarm_service_evt param NULL pointer \n");
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	
	if (alarm_service_struct == NULL)
	{
		ALARM_TRACE_ERROR("Bad ALARM alarm_service_struct param NULL pointer \n");
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	
	if (alarm_service_struct->As_length > 0 )
	{
		alarm_service_struct->As_length--;
		
		if (alarm_service_struct->As_read >= AS_MAX_EVENT_BUFFER_SIZE -1)
		{
			alarm_service_struct->As_read = 0;
		}
		
		*alarm_service_evt = *(alarm_service_struct->Ns_event + (alarm_service_struct->As_read)/**sizeof(struct Ns_event_Str)*/);
		
		alarm_service_struct->As_read++;

	}	
	
	return err;
}

CRED_MW_Errors_t alarm_notification_init(struct Ns_Notify_Str *alarm_service_notify_S)
{

	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;

	if (alarm_service_notify_S == NULL)
	{
		ALARM_TRACE_ERROR("Bad ALARM notify_S param NULL pointer \n");
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
		
	err = alarm_events_init(alarm_service_notify_S);
	if ( err < 0)
	{
		ALARM_TRACE_ERROR("Unable to initialyze ALARM EVENTS driver !!\n");
		return CRED_MW_ERROR_DEVICE;
	}
	
	as_notification_exit = 0;
	err = CRED_TOOLS_CreateTask("AlarmNotifyThread", &alarm_notify_pthread_t, 0x2000,alarm_notify_thread, 1, SCHED_FIFO, (void *)alarm_service_notify_S) ;
	
	if ( err < 0)
	{
		ALARM_TRACE_ERROR("Uneable to create  ALARM Notify thread !!\n");
		return CRED_MW_ERROR_DEVICE;
	}
	
	return err;
}

CRED_MW_Errors_t alarm_service_init(void)
{

	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	
	rf_zone_global_ptr = NULL;
	rf_zone_global_ptr = malloc((sizeof(alarm_service_rf_zone_t))* (MAX_RF_ZONE_NUMBER+2));
	if (rf_zone_global_ptr == NULL)
	{
		ALARM_TRACE_ERROR("Bad ALARM rf_zone_global_ptr param NULL pointer : 0x%x 0x%X\n",rf_zone_global_ptr,((sizeof(alarm_service_rf_zone_t))* (MAX_RF_ZONE_NUMBER+2)));
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	load_rf_zone_information();
	
	rf_output_global_ptr = NULL;
	rf_output_global_ptr = malloc((sizeof(alarm_service_rf_output_t))* (MAX_RF_OUTPUT_NUMBER+2));
	if (rf_output_global_ptr == NULL)
	{
		ALARM_TRACE_ERROR("Bad ALARM rf_output_global_ptr param NULL pointer : 0x%x 0x%X\n",rf_output_global_ptr,((sizeof(alarm_service_rf_output_t))* (MAX_RF_OUTPUT_NUMBER+2)));
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	load_rf_output_information();
	
	rf_remote_global_ptr = NULL;
	rf_remote_global_ptr = malloc((sizeof(alarm_service_rf_remote_t))* (MAX_RF_REMOTE_NUMBER+2));
	if (rf_remote_global_ptr == NULL)
	{
		ALARM_TRACE_ERROR("Bad ALARM rf_remote_global_ptr param NULL pointer : 0x%x 0x%X\n",
		rf_remote_global_ptr,((sizeof(alarm_service_rf_remote_t))* (MAX_RF_REMOTE_NUMBER+2)));
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	load_rf_remote_information();
	
	rf_keypad_global_ptr = NULL;
	rf_keypad_global_ptr = malloc((sizeof(alarm_service_rf_keypad_t))* (MAX_RF_KEYPAD_NUMBER+2));
	if (rf_keypad_global_ptr == NULL)
	{
		ALARM_TRACE_ERROR("Bad ALARM rf_keypad_global_ptr param NULL pointer : 0x%x 0x%X\n",
		rf_keypad_global_ptr,((sizeof(alarm_service_rf_keypad_t))* (MAX_RF_KEYPAD_NUMBER+2)));
		return CRED_MW_ERROR_BAD_PARAMETER;
	}	
	user_code_global_ptr = NULL;
	user_code_global_ptr = malloc((sizeof(alarm_service_user_code_t))* (MAX_USER_CODE+1));
	if (user_code_global_ptr == NULL)
	{
		ALARM_TRACE_ERROR("Bad ALARM user_code_global_ptr param NULL pointer : 0x%x 0x%X\n",
		user_code_global_ptr,((sizeof(alarm_service_user_code_t))* (MAX_USER_CODE+1)));
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	load_user_code_information();
	
	installer_code_global_ptr = NULL;
	installer_code_global_ptr = malloc((sizeof(alarm_service_installer_code_t))* (MAX_INSTALLER_CODE+1));
	if (installer_code_global_ptr == NULL)
	{
		ALARM_TRACE_ERROR("Bad ALARM installer_code_global_ptr param NULL pointer : 0x%x 0x%X\n",
		installer_code_global_ptr,((sizeof(alarm_service_installer_code_t))* (MAX_INSTALLER_CODE+1)));
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	load_installer_code_information();
	
	manufacturer_global_ptr = NULL;
	manufacturer_global_ptr = malloc((sizeof(alarm_service_manufacturer_code_t))* (MAX_MANUFACTURER_CODE+1));
	if (manufacturer_global_ptr == NULL)
	{
		ALARM_TRACE_ERROR("Bad ALARM manufacturer_global_ptr param NULL pointer : 0x%x 0x%X\n",
		manufacturer_global_ptr,((sizeof(alarm_service_manufacturer_code_t))* (MAX_MANUFACTURER_CODE+1)));
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	
	receiver_global_ptr = NULL;
	receiver_global_ptr = malloc((sizeof(alarm_service_receiver_t))* (MAX_RECEIVER_NUMBER+1));
	if (receiver_global_ptr == NULL)
	{
		ALARM_TRACE_ERROR("Bad ALARM receiver_global_ptr param NULL pointer : 0x%x 0x%X\n",
		receiver_global_ptr,((sizeof(alarm_service_receiver_t))* (MAX_RECEIVER_NUMBER+1)));
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	load_receiver_information();
	
	cctv_global_ptr = NULL;
	cctv_global_ptr = malloc((sizeof(alarm_service_cctv_t))* (MAX_CCTV_NUMBER+1));
	if (cctv_global_ptr == NULL)
	{
		ALARM_TRACE_ERROR("Bad ALARM cctv_global_ptr param NULL pointer : 0x%x 0x%X\n",
		cctv_global_ptr,((sizeof(alarm_service_cctv_t))* (MAX_CCTV_NUMBER+1)));
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	load_cctv_information();

	area_global_ptr = NULL;
	area_global_ptr = malloc((sizeof(alarm_service_area_t))* (MAX_AREA_NUMBER+1));
	if (area_global_ptr == NULL)
	{
		ALARM_TRACE_ERROR("Bad ALARM area_global_ptr param NULL pointer : 0x%x 0x%X\n",
		area_global_ptr,((sizeof(alarm_service_area_t))* (MAX_AREA_NUMBER+1)));
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	load_area_information();

	arm_option_global_ptr = NULL;
	arm_option_global_ptr = malloc((sizeof(alarm_service_arm_option_t)));
	if (arm_option_global_ptr == NULL)
	{
		ALARM_TRACE_ERROR("Bad ALARM arm_option_global_ptr param NULL pointer : 0x%x 0x%X\n",
		arm_option_global_ptr,((sizeof(alarm_service_arm_option_t)))*2);
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	load_arm_option_information();
	
	alarm_arm_status_global_ptr = NULL;
	alarm_arm_status_global_ptr = malloc((sizeof(alarm_service_arm_status_t)));
	if (alarm_arm_status_global_ptr == NULL)
	{
		ALARM_TRACE_ERROR("Bad ALARM alarm_arm_status_global_ptr param NULL pointer : 0x%x 0x%X\n",
		alarm_arm_status_global_ptr,((sizeof(alarm_service_arm_status_t))));
		return CRED_MW_ERROR_BAD_PARAMETER;
	}

	alarm_system_info_global_ptr = NULL;
	alarm_system_info_global_ptr = malloc((sizeof(alarm_system_info_t))*2);
	if (alarm_system_info_global_ptr == NULL)
	{
		ALARM_TRACE_ERROR("Bad ALARM alarm_system_info_global_ptr param NULL pointer : 0x%x 0x%X\n",
		alarm_system_info_global_ptr,((sizeof(alarm_system_info_t))));
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	alarm_load_system_info();
	profile_global_ptr = NULL;

	profile_global_ptr = malloc((sizeof(alarm_service_profile_t))* (MAX_PROFILE_NUMBER+1));
	if (profile_global_ptr == NULL)
	{
		ALARM_TRACE_ERROR("Bad ALARM profile_global_ptr param NULL pointer : 0x%x 0x%X\n",
		profile_global_ptr,((sizeof(alarm_service_profile_t))* (MAX_PROFILE_NUMBER+1)));
		return CRED_MW_ERROR_BAD_PARAMETER;
	}

	load_profile_information();
	/* Init alarm service events */
	if(CRED_NO_ERROR!=alarm_notification_init(&alarm_service_notify_S))
	{
		ALARM_TRACE_ERROR("fail to init alarm events system \n");
		return -1;
	}
	rf_device_type_action = DEV_INIT;
	ALARM_TRACE_INFO("Alarm service init done \n");
	alarm_service_init_flag = 1;
	return err;
}

CRED_MW_Errors_t alarm_notification_term(struct Ns_Notify_Str *alarm_service_notify_S)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	
	if (alarm_service_notify_S == NULL)
	{
		ALARM_TRACE_ERROR("Bad ALARM notify_S param NULL pointer \n");
		return CRED_MW_ERROR_BAD_PARAMETER;
	}

	as_notification_exit = 1;

	sem_post(&alarm_service_notify_S->As_sem);
		
	usleep(20000);
	
	err = alarm_events_term(alarm_service_notify_S)	;
	
	if (err < 0)
	{
		ALARM_TRACE_ERROR("Unable to Term ALARM EVENTS driver\n");
		return CRED_MW_ERROR_DEVICE;
	}
	
	return err;
}
CRED_MW_Errors_t alarm_service_term(void)
{

	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	
	alarm_notification_term(&alarm_service_notify_S);
	
	free(rf_zone_global_ptr);
	free(rf_output_global_ptr);
	free(rf_remote_global_ptr);
	free(rf_keypad_global_ptr);
	free(user_code_global_ptr);
	free(installer_code_global_ptr);
	free(receiver_global_ptr);
	free(cctv_global_ptr);
	free(area_global_ptr);
	free(arm_option_global_ptr);
	free(alarm_arm_status_global_ptr);
	free(alarm_system_info_global_ptr);
	free(profile_global_ptr);

	return err;
}

CRED_MW_Errors_t alarm_service_notify(struct Ns_event_Str alarm_service_event_str)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	
	if(alarm_service_init_flag == 0)
	{
		ALARM_TRACE_ERROR("Bad SysEvent param \n");
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	
	if ((alarm_service_event_str.As_event > CRED_ALARM_LAST) ||  (alarm_service_event_str.As_event < 0))
	{
		ALARM_TRACE_ERROR("Bad Pwer Event param \n");
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	
	if (alarm_service_event_tmp == NULL)
	{
		ALARM_TRACE_ERROR("Bad event_tmp param NULL pointer \n");
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	
	if (alarm_service_event_tmp->As_length < AS_MAX_EVENT_BUFFER_SIZE)
	{
		alarm_service_event_tmp->As_length++;
		
		if (alarm_service_event_tmp->As_write >= AS_MAX_EVENT_BUFFER_SIZE -1)
		{
			alarm_service_event_tmp->As_write = 0;
		}
		
		*(alarm_service_event_tmp->Ns_event + (alarm_service_event_tmp->As_write)/**sizeof(struct Ns_event_Str)*/) = alarm_service_event_str;
		
		alarm_service_event_tmp->As_write++;
		
		notify_timestamp(&alarm_service_event_tmp->As_time);
		
		sem_post(&alarm_service_event_tmp->As_sem);
	}	
	
	return err;
}

CRED_MW_Errors_t alarm_rf_detectors_event_Notify(uint8_t *rf_event_detail)
{
	
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	int i=0;
	uint8_t status_mask = 0x01;
	uint8_t *rf_devices_subevent_ptr = NULL,*rf_devices_status_ptr =NULL;
	struct Ns_event_Str alarm_service_event;
	RF_Global_Detector_str_t *rf_global_detector_ptr = (RF_Global_Detector_str_t *)rf_event_detail;
	rf_devices_subevent_ptr = rf_devices_subevent;
	uint8_t RF_New_State,RF_Old_State,alarm_mask;
	
	if(rf_global_detector_ptr == NULL)
	{
		ALARM_TRACE_ERROR("Bad parameter : rf_global_detector_ptr null pointer\n");
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	
	switch(rf_global_detector_ptr->Specific_Device_Type)
	{
		case PIR_DETECTOR:
		{
			rf_devices_status_ptr = pir_detector_status;
			alarm_mask = 0xF4;
			ALARM_TRACE("Alarm service rf devices : PIR_DETECTOR \n");
		}
		break;
		case MAG_DETECTOR:
		{
			rf_devices_status_ptr = magnet_detector_status;
			alarm_mask = 0xD4;
			ALARM_TRACE("Alarm service rf devices : MAG_DETECTOR \n");
		}
		break;
		case SMOKE_DETECTOR:
		{
			rf_devices_status_ptr = pir_detector_status;
			alarm_mask = 0xD4;
			ALARM_TRACE("Alarm service rf devices : SMOKE_DETECTOR \n");
		}
		break;
		case GAS_DETECTOR:
		{
			rf_devices_status_ptr = pir_detector_status;
			alarm_mask = 0xD4;
			ALARM_TRACE("Alarm service rf devices : GAS_DETECTOR \n");
		}
		break;
		case GLASS_BREAK_DETECTOR:
		{
			rf_devices_status_ptr = pir_detector_status;
			alarm_mask = 0xFE;
			ALARM_TRACE("Alarm service rf devices : GLASS_BREAK_DETECTOR \n");
		}
		case PIR_CAM_DETECTOR:
		{
			rf_devices_status_ptr = pir_detector_status;
			alarm_mask = 0xF4;
			ALARM_TRACE("Alarm service rf devices : PIR_CAM_DETECTOR \n");
		}
		break;
		case FLOOD_DETECTOR:
		{
			rf_devices_status_ptr = magnet_detector_status;
			alarm_mask = 0xD4;
			ALARM_TRACE("Alarm service rf devices : FLOOD_DETECTOR \n");
		}
		break;
		case VIBRATION_DETECTOR:
		{
			rf_devices_status_ptr = pir_detector_status;
			alarm_mask = 0xD4;
			ALARM_TRACE("Alarm service rf devices : VIBRATION_DETECTOR \n");
		}
		break;
		default:
				ALARM_TRACE_ERROR("Alarm service unknow rf devices\n");
		break;
	}
	if(rf_devices_status_ptr == NULL)
	{
		ALARM_TRACE_ERROR("Bad parameter : rf_devices_status_ptr null pointer\n");
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	RF_New_State = rf_global_detector_ptr->RF_New_State & alarm_mask;
	RF_Old_State = rf_global_detector_ptr->RF_Old_State & alarm_mask;
	ALARM_TRACE_INFO("Alarm service rf devices status : old  = 0x%x new = 0x%x  \n",RF_Old_State,RF_New_State);
	if(RF_New_State != RF_Old_State)
	{
		for(i=0;i<8;i++)
		{
			if((RF_New_State & status_mask) != (RF_Old_State & status_mask))
			{
				memset(rf_zone_alarm_to_ns_event,0,sizeof(CRED_AS_EVENT_PARAM_t));
				rf_zone_alarm_to_ns_event->source_type = EV_SOURCE_RF_ZONE; /* RF Zone events */
				rf_zone_alarm_to_ns_event->fault_source_id = rf_global_detector_ptr->Zone_index;
				/* new event for RF detectirs */
				if(rf_global_detector_ptr->RF_New_State & status_mask)
				{
					/* restore : back to normal status */
					rf_zone_alarm_to_ns_event->Action = EV_ACTION_RESTORE;
					((alarm_service_rf_zone_t *)(rf_zone_global_ptr + rf_zone_alarm_to_ns_event->fault_source_id))->zone_alarm_status = CRED_ZONE_ALARM_STATUS_NO_ALARM;
				
				}else
				{
					/* alarm status*/
					rf_zone_alarm_to_ns_event->Action = EV_ACTION_NO;
					((alarm_service_rf_zone_t *)(rf_zone_global_ptr + rf_zone_alarm_to_ns_event->fault_source_id))->zone_alarm_status = rf_zone_alarm_status[i];
				}
				if(((alarm_service_rf_zone_t *)(rf_zone_global_ptr + rf_zone_alarm_to_ns_event->fault_source_id))->zone_arm_status == CRED_ZONE_ARM_STATUS_ARMED)
				{
					rf_zone_alarm_to_ns_event->event_type = *(rf_devices_subevent_ptr+i);
					rf_zone_alarm_to_ns_event->event = rf_global_detector_ptr->Zone_type;
					rf_zone_alarm_to_ns_event->area_type = ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + rf_zone_alarm_to_ns_event->fault_source_id))->zone_area; /*EV_AREA_TYPE_AREA_ALL; /* zone area */ /* TBD */
					rf_zone_alarm_to_ns_event->event_detail = *(rf_devices_status_ptr+i); /* RF Zone events */
					alarm_service_event.As_event = CRED_AS_RF_DEVICES;
					alarm_service_event.event_detail =(uint8_t *)rf_zone_alarm_to_ns_event;
					NS_Notify(alarm_service_event);
					rf_zone_to_ns_index++;
					rf_zone_alarm_to_ns_event++;
					ALARM_TRACE_INFO("RF Notify rf_zone_to_ns_index :d \n",rf_zone_to_ns_index);
					if(rf_zone_to_ns_index >= MAX_EVENT_BUFFER_SIZE){
						rf_zone_to_ns_index =0;
						rf_zone_alarm_to_ns_event = rf_zone_alarm_to_ns_Start;
					}
				}
			}
			status_mask =  status_mask <<1;
			ALARM_TRACE_INFO("Rf status_mask :0x%x\n",status_mask);
		}
	}
	
	/* Specific case for Camera PIR detector */
	/* Picture set is pending */
		
	return err;
}

CRED_MW_Errors_t alarm_rf_output_event_Notify(uint8_t *rf_event_detail)
{
	
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	int i=0;
	uint8_t status_mask = 0x01;
	uint8_t *rf_devices_subevent_ptr=NULL,*rf_devices_status_ptr=NULL;
	struct Ns_event_Str alarm_service_event;
	RF_Global_Output_str_t *rf_global_output_ptr = (RF_Global_Output_str_t *)rf_event_detail;
	rf_devices_subevent_ptr = rf_output_subevent;
	uint8_t RF_New_State,RF_Old_State,alarm_mask;
	
	if(rf_global_output_ptr == NULL)
	{
		ALARM_TRACE_ERROR("Bad parameter : rf_devices_status_ptr null pointer\n");
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	
	switch(rf_global_output_ptr->Specific_Device_Type)
	{
		case SIREN_DEVICE:
		{
			rf_devices_status_ptr = output_status;
			alarm_mask = 0xD0;
			ALARM_TRACE("Alarm service rf output devices : SIREN_DEVICE \n");
		}
		break;
		case OUTPUT_DEVICE:
		{
			rf_devices_status_ptr = output_status;
			alarm_mask = 0xD0;
			ALARM_TRACE("Alarm service rf output devices : OUTPUT_DEVICE \n");
		}
		break;
		default:
				ALARM_TRACE_ERROR("Alarm service unknow rf output devices\n");
		break;
	}
	
	if(rf_devices_status_ptr == NULL)
	{
		ALARM_TRACE_ERROR("Bad parameter : rf_devices_status_ptr null pointer\n");
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	
	RF_New_State = rf_global_output_ptr->RF_New_State & alarm_mask;
	RF_Old_State = rf_global_output_ptr->RF_Old_State & alarm_mask;
	ALARM_TRACE_INFO("Alarm service rf output status : old  = 0x%x new = 0x%x  \n",RF_Old_State,RF_New_State);
	if(RF_New_State != RF_Old_State)
	{
		for(i=0;i<8;i++)
		{
			if((RF_New_State & status_mask) != (RF_Old_State & status_mask))
			{
				memset(rf_zone_alarm_to_ns_event,0,sizeof(CRED_AS_EVENT_PARAM_t));
				/* new event for RF detectirs */
				if(rf_global_output_ptr->RF_New_State & status_mask)
				{
					/* restore : back to normal status */
					rf_zone_alarm_to_ns_event->Action = EV_ACTION_RESTORE;
				
				}else
				{
					/* alarm status*/
					rf_zone_alarm_to_ns_event->Action = EV_ACTION_NO;
				}
				rf_zone_alarm_to_ns_event->source_type = EV_SOURCE_RF_OUTPUT; /* RF Zone events */
				rf_zone_alarm_to_ns_event->fault_source_id = rf_global_output_ptr->Output_index;
				rf_zone_alarm_to_ns_event->event_type = *(rf_devices_subevent_ptr+i);
				rf_zone_alarm_to_ns_event->event = rf_global_output_ptr->Output_Type;
				rf_zone_alarm_to_ns_event->area_type = EV_AREA_TYPE_AREA_ALL; /* zone area */ /* TBD */
				rf_zone_alarm_to_ns_event->event_detail = *(rf_devices_status_ptr+i); /* RF Zone events */
				
				alarm_service_event.As_event = CRED_AS_RF_DEVICES;
				alarm_service_event.event_detail =(uint8_t *)rf_zone_alarm_to_ns_event;
				NS_Notify(alarm_service_event);
				
				rf_zone_to_ns_index++;
				rf_zone_alarm_to_ns_event++;
				ALARM_TRACE_INFO("RF Notify rf_zone_to_ns_index :d \n",rf_zone_to_ns_index);
				if(rf_zone_to_ns_index >= MAX_EVENT_BUFFER_SIZE){
					rf_zone_to_ns_index =0;
					rf_zone_alarm_to_ns_event = rf_zone_alarm_to_ns_Start;
				}
			}
					
			status_mask =  status_mask <<1;
			ALARM_TRACE_INFO("Rf status_mask :0x%x\n",status_mask);
		}
	}
	
		
	return err;
}

CRED_MW_Errors_t alarm_rf_pendant_event_Notify(uint8_t *rf_event_detail)
{
	
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	int i=0;
	uint8_t status_mask = 0x01;
	uint8_t *rf_devices_subevent_ptr=NULL,*rf_devices_status_ptr=NULL;
	struct Ns_event_Str alarm_service_event;
	RF_Global_Pendant_str_t *rf_global_pendant_ptr = (RF_Global_Pendant_str_t *)rf_event_detail;
	uint8_t RF_New_State,RF_Old_State,alarm_mask;
	rf_devices_subevent_ptr = rf_devices_subevent;
	
	if(rf_global_pendant_ptr == NULL)
	{
		ALARM_TRACE_ERROR("Bad parameter : rf_global_pendant_ptr null pointer\n");
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	
	switch(rf_global_pendant_ptr->Specific_Device_Type)
	{
		case PENDANT_REMOTE_CONTROL:
		{
				
			if(rf_global_pendant_ptr->Pendant_Type == CRED_RF_ZONE_PANIC)
			{
				ALARM_TRACE("Alarm service rf output devices : PANIC_DEVICE\n");
				rf_devices_status_ptr = panic_status;
				alarm_mask = 0xD0;
			}else
			{
				ALARM_TRACE("Alarm service rf output devices : PENDANT_REMOTE_CONTROL\n");
				rf_devices_status_ptr = remote_status;
				alarm_mask = 0xDF;
			}
		}
		break;
		default:
				ALARM_TRACE_ERROR("Alarm service unknow rf pendant devices\n");
		break;
	}
	if(rf_devices_status_ptr == NULL)
	{
		ALARM_TRACE_ERROR("Bad parameter : rf_devices_status_ptr null pointer\n");
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	
	RF_New_State = rf_global_pendant_ptr->RF_New_State & alarm_mask ;
	RF_Old_State = rf_global_pendant_ptr->RF_Old_State & alarm_mask;
	ALARM_TRACE_INFO("Alarm service rf pendant status : old  = 0x%x new = 0x%x  \n",RF_Old_State,RF_New_State);
	if(rf_global_pendant_ptr->Pendant_Type == CRED_RF_ZONE_PANIC)
	{
		RF_Old_State |= 0x40;
		RF_New_State &= 0xBF;
	}
	
	if(RF_New_State != RF_Old_State)
	{
		for(i=0;i<8;i++)
		{
			if((RF_New_State & status_mask) != (RF_Old_State & status_mask))
			{
				
				if(i > 3 ){
					/* new event for RF detectirs */
					memset(rf_zone_alarm_to_ns_event,0,sizeof(CRED_AS_EVENT_PARAM_t));
					if(rf_global_pendant_ptr->RF_New_State & status_mask)
					{
						/* restore : back to normal status */
						rf_zone_alarm_to_ns_event->Action = EV_ACTION_RESTORE;
					
					}else
					{
						/* alarm status*/
						rf_zone_alarm_to_ns_event->Action = EV_ACTION_NO;
					}
					
					if(rf_global_pendant_ptr->Pendant_Type == CRED_RF_ZONE_PANIC)
					{
						rf_zone_alarm_to_ns_event->source_type = EV_SOURCE_RF_ZONE; /* RF Zone events */
						rf_zone_alarm_to_ns_event->event = CRED_RF_ZONE_PANIC;//rf_global_pendant_ptr->Pendant_Type;
						rf_zone_alarm_to_ns_event->fault_source_id = rf_global_pendant_ptr->Zone_index;
					}else
					{
						rf_zone_alarm_to_ns_event->source_type = EV_SOURCE_RF_REMOTE; /* RF remote events */
						rf_zone_alarm_to_ns_event->event = CRED_RF_REMOTE;
						rf_zone_alarm_to_ns_event->fault_source_id = rf_global_pendant_ptr->Pendant_index;
					}
					
					rf_zone_alarm_to_ns_event->event_type = *(rf_devices_subevent_ptr+i);
					
					rf_zone_alarm_to_ns_event->area_type = EV_AREA_TYPE_AREA_ALL; /* zone area */ /* TBD */
					rf_zone_alarm_to_ns_event->event_detail = *(rf_devices_status_ptr+i); /* RF Zone events */
					
					alarm_service_event.As_event = CRED_AS_RF_DEVICES;
					alarm_service_event.event_detail =(uint8_t *)rf_zone_alarm_to_ns_event;
					NS_Notify(alarm_service_event);
					
					rf_zone_to_ns_index++;
					rf_zone_alarm_to_ns_event++;
					ALARM_TRACE_INFO("RF Notify rf_zone_to_ns_index :d \n",rf_zone_to_ns_index);
					if(rf_zone_to_ns_index >= MAX_EVENT_BUFFER_SIZE){
						rf_zone_to_ns_index =0;
						rf_zone_alarm_to_ns_event = rf_zone_alarm_to_ns_Start;
					}
				}else if(RF_Old_State & status_mask)
				{
					if(status_mask == 1)
					{
						ALARM_TRACE_INFO("RF REMOTE : Button 1 pressed \n");
						disarm_all_system();
						if(output_index_global){
							Set_Siren_Output_Status(output_index_global, 2, 0);
						}
					}else if(status_mask == 2)
					{
						ALARM_TRACE_INFO("RF REMOTE : Button 2 pressed \n");
						arm_all_system();
						if(output_index_global){
							Set_Siren_Output_Status(output_index_global, 3, 0);
						}
					}else if(status_mask == 4)
					{
						ALARM_TRACE_INFO("RF REMOTE : Button 3 pressed \n");
					}else if(status_mask == 8)
					{
						ALARM_TRACE_INFO("RF REMOTE : Button 4 pressed \n");
					}					
				}
			}
					
			status_mask =  status_mask <<1;
			ALARM_TRACE_INFO("Rf status_mask :0x%x\n",status_mask);
		}
	}
	

		
	return err;
}

CRED_MW_Errors_t alarm_rf_module_event_Notify(uint8_t *rf_event_detail)
{
	
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	int i=0;
	uint8_t status_mask = 0x01;
	uint8_t *rf_devices_subevent_ptr=NULL,*rf_devices_status_ptr=NULL;
	struct Ns_event_Str alarm_service_event;
	RF_Module_Info_t *rf_global_module_ptr = (RF_Module_Info_t *)rf_event_detail;
	
	rf_devices_subevent_ptr = rf_devices_subevent;
	
	if(rf_global_module_ptr == NULL)
	{
		ALARM_TRACE_ERROR("Bad parameter : rf_devices_status_ptr null pointer\n");
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	if(rf_global_module_ptr->Jamming != rf_module_jamming)
	{
		
		if(rf_global_module_ptr->Jamming == 0x01)
		{
			rf_zone_alarm_to_ns_event->Action = EV_ACTION_RESTORE; /* Restore : tamper event */
		}else
		{
			rf_zone_alarm_to_ns_event->Action = EV_ACTION_NO; /* New : event */
		}
		rf_zone_alarm_to_ns_event->event_type = EV_TYPE_SABOTAGE_PANEL; /* event type sabotage panel*/
		rf_zone_alarm_to_ns_event->area_type = EV_AREA_TYPE_AREA_ALL; /* Area event */
		rf_zone_alarm_to_ns_event->source_type = EV_SOURCE_PANEL; /* Source control panel */
		rf_zone_alarm_to_ns_event->fault_source_id = 0; /* No ID required for panel */
		rf_zone_alarm_to_ns_event->device_type =EV_SOURCE_PANEL;/* Source of event: control panel */		
		rf_zone_alarm_to_ns_event->device_id = 0; /* No ID required for panel */
		rf_zone_alarm_to_ns_event->event =  EV_SABOTAGE_PANEL_JAMMING_RF_MODULE;  /*Sabotage: Jamming RF module  (M)*/
		rf_zone_alarm_to_ns_event->event_detail = EV_SABOTAGE_PANEL_JAMMING_RF_MODULE;  /*Sabotage: Jamming RF module  (M)*/
		
		alarm_service_event.As_event = CRED_AS_RF_DEVICES;
		alarm_service_event.event_detail =(uint8_t *)rf_zone_alarm_to_ns_event;
		NS_Notify(alarm_service_event);
				
		rf_zone_to_ns_index++;
		rf_zone_alarm_to_ns_event++;
		ALARM_TRACE_INFO("RF Notify rf_zone_to_ns_index :d \n",rf_zone_to_ns_index);
		if(rf_zone_to_ns_index >= MAX_EVENT_BUFFER_SIZE){
			rf_zone_to_ns_index =0;
			rf_zone_alarm_to_ns_event = rf_zone_alarm_to_ns_Start;
		}
		
		rf_module_jamming = rf_global_module_ptr->Jamming;
	}
	if((rf_global_module_ptr->Module_Initialisation != module_Initialisation) )
	{
		
		if(rf_global_module_ptr->Module_Initialisation == 0x00)
		{
			rf_zone_alarm_to_ns_event->Action = EV_ACTION_RESTORE; /* Restore :  event */
		}else
		{
			rf_zone_alarm_to_ns_event->Action = EV_ACTION_NO; /* new :  event */
		}
		rf_zone_alarm_to_ns_event->event_type = EV_TYPE_FAULT_PANEL; /* event type fault panel*/
		rf_zone_alarm_to_ns_event->area_type = EV_AREA_TYPE_AREA_ALL; /* Area event */
		rf_zone_alarm_to_ns_event->source_type = EV_SOURCE_PANEL; /* Source control panel */
		rf_zone_alarm_to_ns_event->fault_source_id = 0; /* No ID required for panel */
		rf_zone_alarm_to_ns_event->device_type =EV_SOURCE_PANEL;/* Source of event: control panel */		
		rf_zone_alarm_to_ns_event->device_id = 0; /* No ID required for panel */
		rf_zone_alarm_to_ns_event->event =  EV_FAULT_PANEL_RF_MODULE;  /*Fault:  RF module  (M)*/
		rf_zone_alarm_to_ns_event->event_detail = EV_FAULT_PANEL_RF_MODULE;  /*Fault: RF module  (M)*/
		
		alarm_service_event.As_event = CRED_AS_RF_DEVICES;
		alarm_service_event.event_detail =(uint8_t *)rf_zone_alarm_to_ns_event;
		NS_Notify(alarm_service_event);
				
		rf_zone_to_ns_index++;
		rf_zone_alarm_to_ns_event++;
		ALARM_TRACE_INFO("RF Notify rf_zone_to_ns_index :d \n",rf_zone_to_ns_index);
		if(rf_zone_to_ns_index >= MAX_EVENT_BUFFER_SIZE){
			rf_zone_to_ns_index =0;
			rf_zone_alarm_to_ns_event = rf_zone_alarm_to_ns_Start;
		}
		
		module_Initialisation = rf_global_module_ptr->Module_Initialisation;
	}
		
	return err;
}
CRED_MW_Errors_t alarm_rf_keypad_event_Notify(uint8_t *rf_event_detail)
{
	
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	int i=0;
	uint8_t status_mask = 0x01;
	uint8_t *rf_devices_subevent_ptr=NULL,*rf_devices_status_ptr=NULL;
	struct Ns_event_Str alarm_service_event;
	RF_Global_Keypad_str_t *rf_keypad_ptr = (RF_Global_Keypad_str_t *)rf_event_detail;
		
	if(rf_keypad_ptr == NULL)
	{
		ALARM_TRACE_ERROR("Bad parameter : rf_global_pendant_ptr null pointer\n");
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	
	switch(rf_keypad_ptr->Specific_Device_Type)
	{
		case KEYPAD_DEVICE:
		{
			ALARM_TRACE("Alarm service rf devices : KEYPAD_DEVICE\n");
		}
		break;
		default:
				ALARM_TRACE_ERROR("Alarm service unknow rf keypad devices\n");
		break;
	}
	return err;
}

CRED_MW_Errors_t alarm_add_rf_zone(uint8_t rf_zone_index, uint32_t *rf_zone_ptr,int unique_rf_id)
{
	
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	
	alarm_service_rf_zone_t *rf_zone_tmp_ptr=NULL;
	ALARM_TRACE_INFO("START: alarm_add_rf_zone rf_zone_index = %d unique_rf_id = %d rf_zone_ptr =0x%x\n", rf_zone_index,unique_rf_id,rf_zone_ptr);
	if((rf_zone_index > MAX_RF_ZONE_NUMBER ) || (rf_zone_index == 0 ))
	{
		ALARM_TRACE_ERROR("unsupported rf zone index: %d \n", rf_zone_index);
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	
	rf_zone_tmp_ptr=((alarm_service_rf_zone_t *)(rf_zone_global_ptr + rf_zone_index));
	if(rf_zone_tmp_ptr->add_status != CRED_RF_DEVICES_NOT_USED)
	{
		ALARM_TRACE_ERROR("rf zone %d already used\n", rf_zone_index);
		*rf_zone_ptr = NULL;
		return CRED_MW_ERROR_BAD_PARAMETER;
	}else
	{
		*rf_zone_ptr = rf_zone_tmp_ptr;
	}
	rf_last_action_str.action_flag = 1; 
	rf_last_action_str.action_rf_type = DETECTOR;
	rf_last_action_str.action_index = rf_zone_index;
	rf_last_action_str.rf_action = CRED_ADD_RF_DEVICES;
	
	rf_zone_tmp_ptr->rf_zone_detector_ptr = NULL;
	rf_zone_tmp_ptr->rf_zone_pendant_ptr = NULL;
	rf_zone_tmp_ptr->zone_index = rf_zone_index;
	rf_zone_tmp_ptr->add_status = CRED_RF_DEVICES_ADD;

	if(unique_rf_id == 0)
	{
		ALARM_TRACE_INFO("Start_Learning_Device:rf_zone_index = %d unique_rf_id = %d rf_zone_tmp_ptr =0x%x\n", rf_zone_index,unique_rf_id,rf_zone_tmp_ptr);
		Start_Learning_Device(DETECTOR, CRED_RF_ZONE_NOT_USED, rf_zone_index);
	}else
	{
		ALARM_TRACE_INFO("RF_Add_Detector - rf_zone_index = %d unique_rf_id = %d rf_zone_tmp_ptr =0x%x\n", rf_zone_index,unique_rf_id,rf_zone_tmp_ptr);
		RF_Add_Detector(rf_zone_index, unique_rf_id);
	}
	ALARM_TRACE_INFO("END: alarm_add_rf_zone rf_zone_index = %d unique_rf_id = %d rf_zone_ptr =0x%x\n", rf_zone_index,unique_rf_id,rf_zone_ptr);
	return err;
}
CRED_MW_Errors_t alarm_add_rf_output(uint8_t rf_output_index, uint32_t *rf_output_ptr,int unique_rf_id)
{
	
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	alarm_service_rf_output_t *rf_output_tmp_ptr=NULL;
	ALARM_TRACE_INFO("Start: alarm_add_rf_output rf_output_index = %d unique_rf_id = %d rf_remote_ptr =0x%x\n", rf_output_index,unique_rf_id,rf_output_ptr);
	if((rf_output_index > MAX_RF_OUTPUT_NUMBER) || (rf_output_index == 0) ){
		ALARM_TRACE_ERROR("unsupported rf output index: %d \n", rf_output_index);
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	
	rf_output_tmp_ptr=((alarm_service_rf_output_t *)(rf_output_global_ptr + rf_output_index));
	if(rf_output_tmp_ptr->add_status != CRED_RF_DEVICES_NOT_USED)
	{
		ALARM_TRACE_ERROR("rf output %d already used\n", rf_output_index);
		*rf_output_ptr = NULL;
		return CRED_MW_ERROR_BAD_PARAMETER;
	}else
	{
		*rf_output_ptr = rf_output_tmp_ptr;
	}
	
	rf_last_action_str.action_flag = 1; 
	rf_last_action_str.action_rf_type = OUTPUT;
	rf_last_action_str.action_index = rf_output_index;
	rf_last_action_str.rf_action = CRED_ADD_RF_DEVICES;
	
	rf_output_tmp_ptr->rf_output_ptr = NULL;
	rf_output_tmp_ptr->output_index = rf_output_index;
	rf_output_tmp_ptr->add_status = CRED_RF_DEVICES_ADD;
	if(unique_rf_id==0)
	{
		Start_Learning_Device(OUTPUT, CRED_OUTPUT_NOT_USED, rf_output_index);
	}else
	{
		RF_Add_Output(rf_output_index, unique_rf_id);
	}
	output_index_global = rf_output_index;
	ALARM_TRACE_INFO("END: alarm_add_rf_output rf_output_index = %d unique_rf_id = %d rf_remote_ptr =0x%x\n", rf_output_index,unique_rf_id,rf_output_ptr);
	return err;
}
CRED_MW_Errors_t alarm_add_rf_remote(uint8_t rf_remote_index, uint32_t *rf_remote_ptr,int unique_rf_id)
{
	
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	alarm_service_rf_remote_t *rf_remote_tmp_ptr=NULL;
	
	if((rf_remote_index > MAX_RF_REMOTE_NUMBER ) || (rf_remote_index < 1) ){
		ALARM_TRACE_ERROR("unsupported rf remote index: %d \n", rf_remote_index);
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	rf_remote_tmp_ptr = ((alarm_service_rf_remote_t *)(rf_remote_global_ptr + rf_remote_index));
	if(rf_remote_tmp_ptr->add_status != CRED_RF_DEVICES_NOT_USED)
	{
		ALARM_TRACE_ERROR("rf remote %d already used\n", rf_remote_index);
		*rf_remote_ptr = NULL;
		ALARM_TRACE_INFO("END: alarm_add_rf_remote rf_remote_index = %d unique_rf_id = %d rf_remote_ptr =0x%x\n", rf_remote_index,unique_rf_id,rf_remote_ptr);
		return CRED_MW_ERROR_BAD_PARAMETER;
	}else
	{
		*rf_remote_ptr = rf_remote_tmp_ptr;
	}
	
	rf_last_action_str.action_flag = 1; 
	rf_last_action_str.action_rf_type = PENDANT;
	rf_last_action_str.action_index = rf_remote_index;
	rf_last_action_str.rf_action = CRED_ADD_RF_DEVICES;
	
	rf_remote_tmp_ptr->rf_remote_ptr = NULL;
	rf_remote_tmp_ptr->remote_index = rf_remote_index;
	rf_remote_tmp_ptr->add_status = CRED_RF_DEVICES_ADD;
	
	if(unique_rf_id == 0)
	{
		Start_Learning_Device(PENDANT, CRED_RF_ZONE_NOT_USED, rf_remote_index);
	}else
	{
		RF_Add_Pendant(rf_remote_index, unique_rf_id);
	}
	ALARM_TRACE_INFO("END: alarm_add_rf_remote rf_remote_index = %d unique_rf_id = %d rf_remote_ptr =0x%x\n", rf_remote_index,unique_rf_id,rf_remote_ptr);
	return err;
}
CRED_MW_Errors_t alarm_delete_rf_zone(uint8_t rf_zone_index, uint32_t *rf_zone_ptr)
{
	
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	alarm_service_rf_zone_t *rf_zone_tmp_ptr=NULL;
	ALARM_TRACE_INFO("START: alarm_delete_rf_zone rf_zone_index = %d rf_zone_ptr =0x%x\n", rf_zone_index,rf_zone_ptr);
	if((rf_zone_index > MAX_RF_ZONE_NUMBER ) || (rf_zone_index == 0 ))
	{
		ALARM_TRACE_ERROR("unsupported rf zone index: %d \n", rf_zone_index);
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	rf_zone_tmp_ptr = ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + rf_zone_index));
	
	if(rf_zone_tmp_ptr->add_status == CRED_RF_DEVICES_NOT_USED)
	{
		ALARM_TRACE_ERROR("rf zone %d already not used\n", rf_zone_index);
		*rf_zone_ptr = NULL;
		return CRED_MW_ERROR_BAD_PARAMETER;
	}else
	{
		*rf_zone_ptr = rf_zone_tmp_ptr;
	}

	rf_last_action_str.action_flag = 1; 
	rf_last_action_str.action_rf_type = DETECTOR;
	rf_last_action_str.action_index = rf_zone_index;
	rf_last_action_str.rf_action = CRED_DELETE_RF_DEVICES;
	//rf_zone_tmp_ptr->rf_zone_detector_ptr = NULL;
	//rf_zone_tmp_ptr->rf_zone_pendant_ptr = NULL;
	rf_zone_tmp_ptr->zone_index = 99;
	rf_zone_tmp_ptr->add_status = CRED_RF_DEVICES_DELETE;
	RF_Delete_Detector(rf_zone_index);
	ALARM_TRACE_INFO("END: alarm_delete_rf_zone rf_zone_index = %d rf_zone_ptr =0x%x\n", rf_zone_index,rf_zone_ptr);
	return err;
}
CRED_MW_Errors_t alarm_delete_rf_output(uint8_t rf_output_index, uint32_t *rf_output_ptr)
{
	
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	alarm_service_rf_output_t *rf_output_tmp_ptr=NULL;
	
	if((rf_output_index > MAX_RF_OUTPUT_NUMBER ) || (rf_output_index == 0) ){
		ALARM_TRACE_ERROR("unsupported rf output index: %d \n", rf_output_index);
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	
	rf_output_tmp_ptr=((alarm_service_rf_output_t *)(rf_output_global_ptr + rf_output_index));
	if(rf_output_tmp_ptr->add_status == CRED_RF_DEVICES_NOT_USED)
	{
		ALARM_TRACE_ERROR("rf output %d already not used\n", rf_output_index);
		*rf_output_ptr = NULL;
		return CRED_MW_ERROR_BAD_PARAMETER;
	}else
	{
		*rf_output_ptr = rf_output_tmp_ptr;
	}
	
	rf_last_action_str.action_flag = 1; 
	rf_last_action_str.action_rf_type = OUTPUT;
	rf_last_action_str.action_index = rf_output_index;
	rf_last_action_str.rf_action = CRED_DELETE_RF_DEVICES;
	//rf_output_tmp_ptr->rf_output_ptr = NULL;
	//rf_output_tmp_ptr->output_index = rf_output_index;
	rf_output_tmp_ptr->add_status = CRED_RF_DEVICES_DELETE;
	RF_Delete_Output(rf_output_index);
	output_index_global = 0;
	return err;
}
CRED_MW_Errors_t alarm_delete_rf_remote(uint8_t rf_remote_index, uint32_t *rf_remote_ptr)
{
	
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	alarm_service_rf_remote_t *rf_remote_tmp_ptr=NULL;
	
	if((rf_remote_index > MAX_RF_REMOTE_NUMBER)||(rf_remote_index ==0) ){
		ALARM_TRACE_ERROR("unsupported rf remote index: %d \n", rf_remote_index);
		return CRED_MW_ERROR_BAD_PARAMETER;
	}

	rf_remote_tmp_ptr = ((alarm_service_rf_remote_t *)(rf_remote_global_ptr + rf_remote_index));
	if(rf_remote_tmp_ptr->add_status == CRED_RF_DEVICES_NOT_USED)
	{
		ALARM_TRACE_ERROR("rf remote %d already not used used\n", rf_remote_index);
		//*rf_remote_ptr = NULL;
		return CRED_MW_ERROR_BAD_PARAMETER;
	}else
	{
		*rf_remote_ptr = rf_remote_tmp_ptr;
	}

	rf_last_action_str.action_flag = 1; 
	rf_last_action_str.action_rf_type = PENDANT;
	rf_last_action_str.action_index = rf_remote_index;
	rf_last_action_str.rf_action = CRED_DELETE_RF_DEVICES;
	
	rf_remote_tmp_ptr->rf_remote_ptr = NULL;
	rf_remote_tmp_ptr->remote_index = rf_remote_index;
	rf_remote_tmp_ptr->add_status = CRED_RF_DEVICES_DELETE;
	
	RF_Delete_Pendant(rf_remote_index);
	return err;
}
/**
 * \fn void alarm_factory_reset_rf_devices(void)
 * \brief Factory reset rf devices.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_factory_reset_rf_devices(void)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	ALARM_TRACE_INFO("START: alarm_factory_reset_rf_devices \n");
	
	rf_last_action_str.action_flag = 1; 
	rf_last_action_str.action_rf_type = DETECTOR;
	rf_last_action_str.action_index = 1;
	rf_last_action_str.rf_action = CRED_RESET_FACTORY_RF_DEVICES;
	RF_Reset_Factory_Start();
	ALARM_TRACE_INFO("END: alarm_factory_reset_rf_devices \n");
	return err;
}

/**
 * \fn void alarm_factory_reset_rf_zone(void)
 * \brief Factory reset rf devices.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_factory_reset_rf_zone(void)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	int i=0;
	ALARM_TRACE_INFO("START: alarm_factory_reset_rf_zone \n");
	
	for(i=1;i<=MAX_RF_ZONE_NUMBER;i++)
	{
		reset_rf_zone_setting(i);
		/*save_rf_zone_information(i);*/
	}
	
	ALARM_TRACE_INFO("END: alarm_factory_reset_rf_zone \n");
	return err;
}
/**
 * \fn void alarm_factory_reset_rf_output(void)
 * \brief Factory reset rf devices.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_factory_reset_rf_output(void)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	int i=0;
	ALARM_TRACE_INFO("START: alarm_factory_reset_rf_output \n");
	
	for(i=1;i<=MAX_RF_OUTPUT_NUMBER;i++)
	{
		reset_rf_output_setting(i);
		save_rf_output_information(i);
	}
	
	ALARM_TRACE_INFO("END: alarm_factory_reset_rf_output \n");
	return err;
}
/**
 * \fn void alarm_factory_reset_rf_remote(void)
 * \brief Factory reset rf devices.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_factory_reset_rf_remote(void)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	int i=0;
	ALARM_TRACE_INFO("START: alarm_factory_reset_rf_remote \n");
	
	for(i=1;i<=MAX_RF_OUTPUT_NUMBER;i++)
	{
		reset_rf_reemote_setting(i);
		save_rf_remote_information(i);
	}
	
	ALARM_TRACE_INFO("END: alarm_factory_reset_rf_remote \n");
	return err;
}

/**
 * \fn void alarm_factory_reset_all(void)
 * \brief Factory reset for all alarm system.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_factory_reset_all(void)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	ALARM_TRACE_INFO("START: alarm_factory_reset_all \n");
	alarm_factory_reset_rf_zone();
	alarm_factory_reset_rf_output();
	alarm_factory_reset_rf_remote();
	alarm_factory_reset_receiver();
	alarm_factory_reset_user();
	alarm_factory_reset_installer();
	alarm_factory_reset_cctv();
	alarm_factory_reset_area();
	reset_arm_option_information();
	alarm_factory_reset_profile();

	PCA9535_SetLedValue(CRED_LED_RED, 1);
	PCA9535_SetLedValue(CRED_LED_GREEN, 1);
	PCA9535_SetLedValue(CRED_LED_ORANGE, 1);

	ALARM_TRACE_INFO("END: alarm_factory_reset_all \n");
	ALARM_TRACE_INFO("Start Rebooting.... \n");

	system("reboot");
	return err;
}

/**
 * \fn void alarm_system_reboot(void)
 * \brief Alarm System reboot.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_system_reboot(void)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	ALARM_TRACE_INFO("Start Rebooting.... \n");
	PCA9535_SetLedValue(CRED_LED_RED, 1);
	PCA9535_SetLedValue(CRED_LED_GREEN, 1);
	PCA9535_SetLedValue(CRED_LED_ORANGE, 1);
	system("reboot");
	return err;
}


CRED_MW_Errors_t alarm_rf_action_event(uint8_t *rf_event_detail)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	uint8_t rf_tmp_index =0;
	alarm_service_rf_zone_t *rf_zone_tmp_ptr=NULL;
	alarm_service_rf_output_t *rf_output_tmp_ptr=NULL;
	alarm_service_rf_remote_t *rf_remote_tmp_ptr=NULL;
	alarm_service_rf_keypad_t *rf_keypad_tmp_ptr=NULL;
	alarm_service_rf_action_t *alarm_serice_rf_action_event_ptr = (alarm_service_rf_action_t *)rf_event_detail;
	ALARM_TRACE_INFO("START: alarm_rf_action_event \n");
	if(alarm_serice_rf_action_event_ptr == NULL)
	{
		ALARM_TRACE_ERROR("Bad parameter : alarm_serice_rf_action_event_ptr null pointer\n");
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	
	//rf_device_type_action = DETECTOR; /* For testing only */
	//rf_last_action_str.action_rf_type = DETECTOR;
	rf_tmp_index = rf_last_action_str.action_index; 
	if(alarm_serice_rf_action_event_ptr->rf_device_type == DEV_INIT)
	{
		alarm_serice_rf_action_event_ptr->rf_device_type = rf_last_action_str.action_rf_type;
		alarm_serice_rf_action_event_ptr->rf_action = rf_last_action_str.rf_action;
		ALARM_TRACE("Alarm service devices : DEV_INIT : last rf devices :%d last action %d\n", rf_last_action_str.action_rf_type,rf_last_action_str.rf_action);
	}
	switch(/*rf_last_action_str.action_rf_type*/alarm_serice_rf_action_event_ptr->rf_device_type)
	{
		case DETECTOR:
		{
			ALARM_TRACE("Alarm service rf devices : DETECTOR\n");
			//rf_tmp_index = alarm_serice_rf_action_event_ptr->rf_detector_ptr->Zone_index;
			
			rf_zone_tmp_ptr = rf_zone_global_ptr + rf_tmp_index;
			if((rf_tmp_index > MAX_RF_ZONE_NUMBER) || (rf_tmp_index == 0) ){
					ALARM_TRACE_ERROR("unsupported rf zone index: %d \n", rf_tmp_index);
					return CRED_MW_ERROR_BAD_PARAMETER;
			}
			if((alarm_serice_rf_action_event_ptr->rf_action == CRED_ADD_RF_DEVICES) ||(alarm_serice_rf_action_event_ptr->rf_action == CRED_LEARN_RF_DEVICES))
			{
				ALARM_TRACE("CRED_ADD_RF_DEVICES\n");
				if(alarm_serice_rf_action_event_ptr->rf_action_result == CRED_RF_ACTION_RESULT_ACK)
				{
					rf_tmp_index = alarm_serice_rf_action_event_ptr->rf_detector_ptr->Zone_index;
					ALARM_TRACE("CRED_RF_ACTION_RESULT_ACK : zone rf index = %d\n",rf_tmp_index);
					rf_zone_tmp_ptr = rf_zone_global_ptr + rf_tmp_index;
					if((rf_tmp_index > MAX_RF_ZONE_NUMBER ) || (rf_tmp_index == 0 )){
						ALARM_TRACE_ERROR("unsupported rf zone index: %d \n", rf_tmp_index);
						return CRED_MW_ERROR_BAD_PARAMETER;
					}
					rf_zone_tmp_ptr->rf_zone_detector_ptr = alarm_serice_rf_action_event_ptr->rf_detector_ptr;
					rf_zone_tmp_ptr->zone_index = rf_tmp_index;
					rf_zone_tmp_ptr->zone_id = alarm_serice_rf_action_event_ptr->rf_detector_ptr->Unique_RF_ID;
					rf_zone_tmp_ptr->add_status = CRED_RF_DEVICES_ADDED;
					ALARM_TRACE("CRED_RF_ACTION_RESULT_ACK : CRED_RF_DEVICES_ADDED\n");
					alarm_rf_zone_added(rf_tmp_index);
				}else if(alarm_serice_rf_action_event_ptr->rf_action_result == CRED_RF_ACTION_RESULT_TIMEOUT)
				{
					rf_zone_tmp_ptr->rf_zone_detector_ptr = NULL;
					rf_zone_tmp_ptr->add_status = CRED_RF_DEVICES_NOT_USED;
					ALARM_TRACE("CRED_RF_DEVICES_ADD_TIMEOUT : CRED_RF_DEVICES_ADD_TIMEOUT\n");
					reset_rf_zone_setting(rf_tmp_index);
				}else if(alarm_serice_rf_action_event_ptr->rf_action_result == CRED_RF_ACTION_RESULT_CANCELED)
				{
					rf_zone_tmp_ptr->rf_zone_detector_ptr = NULL;
					rf_zone_tmp_ptr->add_status = CRED_RF_DEVICES_NOT_USED;
					reset_rf_zone_setting(rf_tmp_index);
					ALARM_TRACE("CRED_RF_ACTION_RESULT_CANCELED : CRED_RF_DEVICES_ADD_CANCELED\n");
				}else if(alarm_serice_rf_action_event_ptr->rf_action_result == CRED_RF_ACTION_RESULT_NACK)
				{
					rf_zone_tmp_ptr->rf_zone_detector_ptr = NULL;
					rf_zone_tmp_ptr->add_status = CRED_RF_DEVICES_NOT_USED;
					reset_rf_zone_setting(rf_tmp_index);
					ALARM_TRACE("CRED_RF_ACTION_RESULT_NACK : CRED_RF_DEVICES_ADD_TIMEOUT\n");
				}
				
			}else if(alarm_serice_rf_action_event_ptr->rf_action == CRED_DELETE_RF_DEVICES)
			{
				ALARM_TRACE("CRED_DELETE_RF_DEVICES\n");
				if(alarm_serice_rf_action_event_ptr->rf_action_result == CRED_RF_ACTION_RESULT_ACK)
				{
					//rf_zone_tmp_ptr->rf_zone_detector_ptr = NULL;
					//rf_zone_tmp_ptr->zone_index = rf_tmp_index;
					//rf_zone_tmp_ptr->zone_id = 0;
					//rf_zone_tmp_ptr->zone_type = 99;
					//rf_zone_tmp_ptr->add_status = CRED_RF_DEVICES_NOT_USED;
					reset_rf_zone_setting(rf_tmp_index);
					ALARM_TRACE("CRED_RF_ACTION_RESULT_ACK : CRED_RF_DEVICES_DELETED rf zone index = %d \n",rf_tmp_index);
				}else if(alarm_serice_rf_action_event_ptr->rf_action_result == CRED_RF_ACTION_RESULT_TIMEOUT)
				{
					rf_zone_tmp_ptr->add_status = CRED_RF_DEVICES_ADDED;
					save_rf_zone_information(rf_tmp_index);
					ALARM_TRACE("CRED_RF_DEVICES_ADD_TIMEOUT : CRED_RF_DEVICES_DELETE_TIMEOUT\n");
				}else if(alarm_serice_rf_action_event_ptr->rf_action_result == CRED_RF_ACTION_RESULT_CANCELED)
				{
					rf_zone_tmp_ptr->add_status = CRED_RF_DEVICES_ADDED;
					save_rf_zone_information(rf_tmp_index);
					ALARM_TRACE("CRED_RF_ACTION_RESULT_CANCELED : CRED_RF_DEVICES_DELETE_CANCELED\n");
				}else if(alarm_serice_rf_action_event_ptr->rf_action_result == CRED_RF_ACTION_RESULT_NACK)
				{
					rf_zone_tmp_ptr->add_status = CRED_RF_DEVICES_ADDED;
					save_rf_zone_information(rf_tmp_index);
					ALARM_TRACE("CRED_RF_ACTION_RESULT_NACK : CRED_RF_DEVICES_DELETE_TIMEOUT\n");
				}
			}else if(alarm_serice_rf_action_event_ptr->rf_action == CRED_CONFIG_RF_DEVICES)
			{
				ALARM_TRACE("CRED_CONFIG_RF_DEVICES\n");
				
			}else if(alarm_serice_rf_action_event_ptr->rf_action == CRED_RESET_FACTORY_RF_DEVICES)
			{
				ALARM_TRACE("CRED_RESET_FACTORY_RF_DEVICES\n");
				if(alarm_serice_rf_action_event_ptr->rf_action_result == CRED_RF_ACTION_RESULT_ACK)
				{
					alarm_factory_reset_all();
					ALARM_TRACE("CRED_RF_ACTION_RESULT_ACK : CRED_RESET_FACTORY_RF_DEVICES \n");
				}else/*if(alarm_serice_rf_action_event_ptr->rf_action_result == CRED_RF_ACTION_RESULT_NACK)*/
				{
				
					ALARM_TRACE("CRED_RF_ACTION_RESULT_NACK : CRED_RESET_FACTORY_RF_DEVICES\n");
				}
				
			}else
			{
				ALARM_TRACE("RF unknow rf action\n");
			}
			rf_last_action_str.action_flag = 0;
			rf_last_action_str.action_rf_type = DEV_INIT;
		}
		break;
		case OUTPUT:
		{
			ALARM_TRACE("Alarm service rf devices : OUTPUT\n");
			//rf_tmp_index = alarm_serice_rf_action_event_ptr->rf_output_ptr->Output_index;
			rf_output_tmp_ptr = rf_output_global_ptr + rf_tmp_index;
			if((rf_tmp_index > MAX_RF_OUTPUT_NUMBER ) || (rf_tmp_index == 0) ){
					ALARM_TRACE_ERROR("unsupported rf output index: %d \n", rf_tmp_index);
					return CRED_MW_ERROR_BAD_PARAMETER;
			}
			if((alarm_serice_rf_action_event_ptr->rf_action == CRED_ADD_RF_DEVICES) ||(alarm_serice_rf_action_event_ptr->rf_action == CRED_LEARN_RF_DEVICES))
			{
				ALARM_TRACE("CRED_ADD_RF_DEVICES\n");
				if(alarm_serice_rf_action_event_ptr->rf_action_result == CRED_RF_ACTION_RESULT_ACK)
				{
					
					rf_tmp_index = alarm_serice_rf_action_event_ptr->rf_output_ptr->Output_index;
					rf_output_tmp_ptr=((alarm_service_rf_output_t *)(rf_output_global_ptr + rf_tmp_index));
					if((rf_tmp_index > MAX_RF_OUTPUT_NUMBER) || (rf_tmp_index==0) )
					{
						ALARM_TRACE_ERROR("unsupported rf output index: %d \n", rf_tmp_index);
						return CRED_MW_ERROR_BAD_PARAMETER;
					}
					rf_output_tmp_ptr->rf_output_ptr = alarm_serice_rf_action_event_ptr->rf_output_ptr;
					rf_output_tmp_ptr->output_index = rf_tmp_index;
					rf_output_tmp_ptr->output_id = alarm_serice_rf_action_event_ptr->rf_output_ptr->Unique_RF_ID;
					rf_output_tmp_ptr->add_status = CRED_RF_DEVICES_ADDED;
					save_rf_output_information(rf_tmp_index);
					ALARM_TRACE("CRED_RF_ACTION_RESULT_ACK:CRED_RF_DEVICES_ADDED\n");	
				}else if(alarm_serice_rf_action_event_ptr->rf_action_result == CRED_RF_ACTION_RESULT_TIMEOUT)
				{
					rf_output_tmp_ptr->rf_output_ptr = NULL;
					rf_output_tmp_ptr->add_status = CRED_RF_DEVICES_NOT_USED;
					save_rf_output_information(rf_tmp_index);
					ALARM_TRACE("CRED_RF_ACTION_RESULT_TIMEOUT:CRED_RF_DEVICES_ADD_TIMEOUT\n");
				}else if(alarm_serice_rf_action_event_ptr->rf_action_result == CRED_RF_ACTION_RESULT_CANCELED)
				{
					rf_output_tmp_ptr->rf_output_ptr = NULL;
					rf_output_tmp_ptr->add_status = CRED_RF_DEVICES_NOT_USED;
					save_rf_output_information(rf_tmp_index);
					ALARM_TRACE("CRED_RF_ACTION_RESULT_CANCELED : CRED_RF_DEVICES_ADD_CANCELED\n");
				}else if(alarm_serice_rf_action_event_ptr->rf_action_result == CRED_RF_ACTION_RESULT_NACK)
				{
					rf_output_tmp_ptr->rf_output_ptr = NULL;
					rf_output_tmp_ptr->add_status = CRED_RF_DEVICES_NOT_USED;
					save_rf_output_information(rf_tmp_index);
					ALARM_TRACE("CRED_RF_ACTION_RESULT_NACK:CRED_RF_DEVICES_ADD_TIMEOUT\n");
				}
				
			}else if(alarm_serice_rf_action_event_ptr->rf_action == CRED_DELETE_RF_DEVICES)
			{
				ALARM_TRACE("CRED_DELETE_RF_DEVICES\n");
				if(alarm_serice_rf_action_event_ptr->rf_action_result == CRED_RF_ACTION_RESULT_ACK)
				{
					rf_output_tmp_ptr->rf_output_ptr = NULL;
					rf_output_tmp_ptr->output_index = rf_tmp_index;
					rf_output_tmp_ptr->output_id = 0;
					rf_output_tmp_ptr->output_type = 99;
					rf_output_tmp_ptr->add_status = CRED_RF_DEVICES_NOT_USED;
					save_rf_output_information(rf_tmp_index);
					ALARM_TRACE("CRED_RF_ACTION_RESULT_ACK:CRED_RF_DEVICES_DELETED\n");
				}else if(alarm_serice_rf_action_event_ptr->rf_action_result == CRED_RF_ACTION_RESULT_TIMEOUT)
				{
					rf_output_tmp_ptr->add_status = CRED_RF_DEVICES_ADDED;
					save_rf_output_information(rf_tmp_index);
					ALARM_TRACE("CRED_RF_DEVICES_ADD_TIMEOUT:CRED_RF_ACTION_RESULT_TIMEOUT\n");
				}else if(alarm_serice_rf_action_event_ptr->rf_action_result == CRED_RF_ACTION_RESULT_CANCELED)
				{
					rf_output_tmp_ptr->add_status = CRED_RF_DEVICES_ADDED;
					save_rf_output_information(rf_tmp_index);
					ALARM_TRACE("CRED_RF_ACTION_RESULT_CANCELED:CRED_RF_ACTION_RESULT_CANCELED\n");
				}else if(alarm_serice_rf_action_event_ptr->rf_action_result == CRED_RF_ACTION_RESULT_NACK)
				{
					rf_output_tmp_ptr->add_status = CRED_RF_DEVICES_ADDED;
					save_rf_output_information(rf_tmp_index);
					ALARM_TRACE("CRED_RF_ACTION_RESULT_NACK:CRED_RF_ACTION_RESULT_NACK\n");
				}
			}else if(alarm_serice_rf_action_event_ptr->rf_action == CRED_CONFIG_RF_DEVICES)
			{
				ALARM_TRACE("CRED_CONFIG_RF_DEVICES\n");
				
			}
			rf_last_action_str.action_flag = 0;
			rf_last_action_str.action_rf_type = DEV_INIT;
			
		}
		break;
		case PENDANT:
		{
			ALARM_TRACE("Alarm service rf devices : PENDANT\n");
			rf_remote_tmp_ptr = (alarm_service_rf_remote_t *)(rf_remote_global_ptr + rf_tmp_index);
			if((rf_tmp_index > MAX_RF_REMOTE_NUMBER)||(rf_tmp_index ==0) ){
					ALARM_TRACE_ERROR("unsupported rf remote index: %d \n", rf_tmp_index);
					return CRED_MW_ERROR_BAD_PARAMETER;
			}
			if((alarm_serice_rf_action_event_ptr->rf_action == CRED_ADD_RF_DEVICES) ||(alarm_serice_rf_action_event_ptr->rf_action == CRED_LEARN_RF_DEVICES))
			{
				ALARM_TRACE("CRED_ADD_RF_DEVICES\n");
				if(alarm_serice_rf_action_event_ptr->rf_action_result == CRED_RF_ACTION_RESULT_ACK)
				{
					rf_tmp_index = alarm_serice_rf_action_event_ptr->rf_pendant_ptr->Remote_index;
					rf_remote_tmp_ptr = (alarm_service_rf_remote_t *)(rf_remote_global_ptr + rf_tmp_index);
					ALARM_TRACE("CRED_RF_ACTION_RESULT_ACK : Remote rf index = %d rf_remote_tmp_ptr = 0x%x\n",rf_tmp_index,rf_remote_tmp_ptr);
					if((rf_tmp_index > MAX_RF_REMOTE_NUMBER) || (rf_tmp_index == 0) ){
						ALARM_TRACE_ERROR("unsupported rf remote index: %d \n", rf_tmp_index);
						return CRED_MW_ERROR_BAD_PARAMETER;
					}
					rf_remote_tmp_ptr->rf_remote_ptr = alarm_serice_rf_action_event_ptr->rf_pendant_ptr;
					rf_remote_tmp_ptr->remote_id = alarm_serice_rf_action_event_ptr->rf_pendant_ptr->Unique_RF_ID;
					rf_remote_tmp_ptr->remote_index = rf_tmp_index;
					rf_remote_tmp_ptr->add_status = CRED_RF_DEVICES_ADDED;
					ALARM_TRACE("CRED_RF_ACTION_RESULT_ACK:CRED_RF_DEVICES_ADDED\n");
					save_rf_remote_information(rf_tmp_index);
				}else if(alarm_serice_rf_action_event_ptr->rf_action_result == CRED_RF_ACTION_RESULT_TIMEOUT)
				{
					rf_remote_tmp_ptr->rf_remote_ptr = NULL;
					rf_remote_tmp_ptr->add_status = CRED_RF_DEVICES_NOT_USED;
					ALARM_TRACE("CRED_RF_ACTION_RESULT_TIMEOUT:CRED_RF_DEVICES_ADD_TIMEOUT\n");
					save_rf_remote_information(rf_tmp_index);
				}else if(alarm_serice_rf_action_event_ptr->rf_action_result == CRED_RF_ACTION_RESULT_CANCELED)
				{
					rf_remote_tmp_ptr->rf_remote_ptr = NULL;
					rf_remote_tmp_ptr->add_status = CRED_RF_DEVICES_NOT_USED;
					save_rf_remote_information(rf_tmp_index);
					ALARM_TRACE("CRED_RF_ACTION_RESULT_CANCELED:CRED_RF_DEVICES_ADD_CANCELED\n");
				}else if(alarm_serice_rf_action_event_ptr->rf_action_result == CRED_RF_ACTION_RESULT_NACK)
				{
					rf_remote_tmp_ptr->rf_remote_ptr = NULL;
					rf_remote_tmp_ptr->add_status = CRED_RF_DEVICES_NOT_USED;
					save_rf_remote_information(rf_tmp_index);
					ALARM_TRACE("CRED_RF_ACTION_RESULT_NACK:CRED_RF_DEVICES_ADD_TIMEOUT\n");
				}
				
			}else if(alarm_serice_rf_action_event_ptr->rf_action == CRED_DELETE_RF_DEVICES)
			{
				ALARM_TRACE("CRED_DELETE_RF_DEVICES\n");
				if(alarm_serice_rf_action_event_ptr->rf_action_result == CRED_RF_ACTION_RESULT_ACK)
				{
					rf_remote_tmp_ptr->rf_remote_ptr = NULL;
					rf_remote_tmp_ptr->remote_index = rf_tmp_index;
					rf_remote_tmp_ptr->remote_id = 0;
					rf_remote_tmp_ptr->remote_type = 99;
					rf_remote_tmp_ptr->add_status = CRED_RF_DEVICES_NOT_USED;
					save_rf_remote_information(rf_tmp_index);
					ALARM_TRACE("CRED_RF_ACTION_RESULT_ACK:CRED_RF_DEVICES_DELETED\n");
				}else if(alarm_serice_rf_action_event_ptr->rf_action_result == CRED_RF_ACTION_RESULT_TIMEOUT)
				{
					rf_remote_tmp_ptr->add_status = CRED_RF_DEVICES_ADDED;
					save_rf_remote_information(rf_tmp_index);
					ALARM_TRACE("CRED_RF_DEVICES_ADD_TIMEOUT:CRED_RF_DEVICES_DELETE_TIMEOUT\n");
				}else if(alarm_serice_rf_action_event_ptr->rf_action_result == CRED_RF_ACTION_RESULT_CANCELED)
				{
					rf_remote_tmp_ptr->add_status = CRED_RF_DEVICES_ADDED;
					save_rf_remote_information(rf_tmp_index);
					ALARM_TRACE("CRED_RF_ACTION_RESULT_CANCELED:CRED_RF_DEVICES_DELETE_CANCELED\n");
				}else if(alarm_serice_rf_action_event_ptr->rf_action_result == CRED_RF_ACTION_RESULT_NACK)
				{
					rf_remote_tmp_ptr->add_status = CRED_RF_DEVICES_ADDED;
					save_rf_remote_information(rf_tmp_index);
					ALARM_TRACE("CRED_RF_ACTION_RESULT_NACK:CRED_RF_DEVICES_DELETE_TIMEOUT\n");
				}
			}else if(alarm_serice_rf_action_event_ptr->rf_action == CRED_CONFIG_RF_DEVICES)
			{
				ALARM_TRACE("CRED_CONFIG_RF_DEVICES\n");
				
			}
			rf_last_action_str.action_flag = 0;
			rf_last_action_str.action_rf_type = DEV_INIT;
		}
		break;
		case KEYPAD:
		{
			ALARM_TRACE("Alarm service rf devices : KEYPAD\n");
			rf_keypad_tmp_ptr = rf_keypad_global_ptr + rf_tmp_index;
			if(rf_tmp_index > MAX_RF_KEYPAD_NUMBER ){
					ALARM_TRACE_ERROR("unsupported rf keypad index: %d \n", rf_tmp_index);
					return CRED_MW_ERROR_BAD_PARAMETER;
			}
			if((alarm_serice_rf_action_event_ptr->rf_action == CRED_ADD_RF_DEVICES) ||(alarm_serice_rf_action_event_ptr->rf_action == CRED_LEARN_RF_DEVICES))
			{
				ALARM_TRACE("CRED_ADD_RF_DEVICES\n");
				if(alarm_serice_rf_action_event_ptr->rf_action_result == CRED_RF_ACTION_RESULT_ACK)
				{
					rf_tmp_index = alarm_serice_rf_action_event_ptr->rf_keypad_ptr->keypad_index;
					rf_keypad_tmp_ptr = rf_keypad_global_ptr + rf_tmp_index;
					if(rf_tmp_index > MAX_RF_KEYPAD_NUMBER ){
						ALARM_TRACE_ERROR("unsupported rf keypad index: %d \n", rf_tmp_index);
						return CRED_MW_ERROR_BAD_PARAMETER;
					}
					rf_keypad_tmp_ptr->rf_keypad_ptr = alarm_serice_rf_action_event_ptr->rf_keypad_ptr;
					rf_keypad_tmp_ptr->add_status = CRED_RF_DEVICES_ADDED;
					ALARM_TRACE("CRED_RF_ACTION_RESULT_ACK\n");	
				}else if(alarm_serice_rf_action_event_ptr->rf_action_result == CRED_RF_ACTION_RESULT_TIMEOUT)
				{
					rf_keypad_tmp_ptr->rf_keypad_ptr = NULL;
					rf_keypad_tmp_ptr->add_status = CRED_RF_DEVICES_NOT_USED;
					ALARM_TRACE("CRED_RF_DEVICES_ADD_TIMEOUT\n");
				}else if(alarm_serice_rf_action_event_ptr->rf_action_result == CRED_RF_ACTION_RESULT_CANCELED)
				{
					rf_keypad_tmp_ptr->rf_keypad_ptr = NULL;
					rf_keypad_tmp_ptr->add_status = CRED_RF_DEVICES_NOT_USED;
					ALARM_TRACE("CRED_RF_ACTION_RESULT_CANCELED\n");
				}else if(alarm_serice_rf_action_event_ptr->rf_action_result == CRED_RF_ACTION_RESULT_NACK)
				{
					rf_keypad_tmp_ptr->rf_keypad_ptr = NULL;
					rf_keypad_tmp_ptr->add_status = CRED_RF_DEVICES_NOT_USED;
					ALARM_TRACE("CRED_RF_ACTION_RESULT_NACK\n");
				}
				
			}else if(alarm_serice_rf_action_event_ptr->rf_action == CRED_DELETE_RF_DEVICES)
			{
				ALARM_TRACE("CRED_DELETE_RF_DEVICES\n");
				if(alarm_serice_rf_action_event_ptr->rf_action_result == CRED_RF_ACTION_RESULT_ACK)
				{
					rf_keypad_tmp_ptr->rf_keypad_ptr = NULL;
					rf_keypad_tmp_ptr->keypad_index = 99;
					//rf_keypad_tmp_ptr->remote_type = 99;
					rf_keypad_tmp_ptr->add_status = CRED_RF_DEVICES_NOT_USED;
					ALARM_TRACE("CRED_RF_ACTION_RESULT_ACK\n");
				}else if(alarm_serice_rf_action_event_ptr->rf_action_result == CRED_RF_ACTION_RESULT_TIMEOUT)
				{
					rf_keypad_tmp_ptr->add_status = CRED_RF_DEVICES_ADDED;
					ALARM_TRACE("CRED_RF_DEVICES_ADD_TIMEOUT\n");
				}else if(alarm_serice_rf_action_event_ptr->rf_action_result == CRED_RF_ACTION_RESULT_CANCELED)
				{
					rf_keypad_tmp_ptr->add_status = CRED_RF_DEVICES_ADDED;
					ALARM_TRACE("CRED_RF_ACTION_RESULT_CANCELED\n");
				}else if(alarm_serice_rf_action_event_ptr->rf_action_result == CRED_RF_ACTION_RESULT_NACK)
				{
					rf_keypad_tmp_ptr->add_status = CRED_RF_DEVICES_ADDED;
					ALARM_TRACE("CRED_RF_ACTION_RESULT_NACK\n");
				}
			}else if(alarm_serice_rf_action_event_ptr->rf_action == CRED_CONFIG_RF_DEVICES)
			{
				ALARM_TRACE("CRED_CONFIG_RF_DEVICES\n");
				
			}
			rf_last_action_str.action_flag = 0;
			rf_last_action_str.action_rf_type = DEV_INIT;
		}
		break;
		case DEV_INIT:
		{
			ALARM_TRACE("Alarm service rf devices : DEV_INIT\n");
			rf_last_action_str.action_flag = 0;
			rf_last_action_str.action_rf_type = DEV_INIT;
		}
		break;
		default:
			ALARM_TRACE_ERROR("Alarm service unknow rf devices type : %d\n",alarm_serice_rf_action_event_ptr->rf_device_type);
		break;
	}
	ALARM_TRACE_INFO("END: alarm_rf_action_event \n");
	return err;
}

/**
 * \fn CRED_MW_Errors_t stop_rf_device_action()
 * \brief stop: add, delete and configuire rf device.
 *
 * \param uint8_t rf_device_index: rf device index(zone, output, remote and keypad )
 * \return No parameter.
 */
CRED_MW_Errors_t stop_rf_device_action(uint8_t rf_device_index)
{
	
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	if((rf_device_index > MAX_RF_ZONE_NUMBER ) || (rf_device_index == 0) ){
		ALARM_TRACE_ERROR("unsupported rf zone index: %d \n", rf_device_index);
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	ALARM_TRACE_INFO("START: stop_rf_device_action rf_device_index = %d \n", rf_device_index);
	rf_last_action_str.action_flag = 1; 
	rf_last_action_str.action_rf_type = DETECTOR;
	rf_last_action_str.action_index = rf_device_index;
	rf_last_action_str.rf_action = CRED_ADD_RF_DEVICES;
	Stop_Learning_Device();
	ALARM_TRACE_INFO("END: stop_rf_device_action rf_device_index = %d \n", rf_device_index);
	return err;
}


/**
 * \fn CRED_MW_Errors_t stop_rf_output_action()
 * \brief stop: add, delete and configuire rf device.
 *
 * \param uint8_t rf_device_index: rf device index(zone, output, remote and keypad )
 * \return No parameter.
 */
CRED_MW_Errors_t stop_rf_output_action(uint8_t rf_device_index)
{
	
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	if((rf_device_index > MAX_RF_ZONE_NUMBER ) || (rf_device_index == 0) ){
		ALARM_TRACE_ERROR("unsupported rf zone index: %d \n", rf_device_index);
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	ALARM_TRACE_INFO("START: stop_rf_device_action rf_device_index = %d \n", rf_device_index);
	rf_last_action_str.action_flag = 1; 
	rf_last_action_str.action_rf_type = OUTPUT;
	rf_last_action_str.action_index = rf_device_index;
	rf_last_action_str.rf_action = CRED_ADD_RF_DEVICES;
	Stop_Learning_Device();
	ALARM_TRACE_INFO("END: stop_rf_device_action rf_device_index = %d \n", rf_device_index);
	return err;
}
/**
 * \fn CRED_MW_Errors_t stop_rf_remote_action()
 * \brief stop: add, delete and configuire rf device.
 *
 * \param uint8_t rf_device_index: rf device index(zone, output, remote and keypad )
 * \return No parameter.
 */
CRED_MW_Errors_t stop_rf_remote_action(uint8_t rf_device_index)
{
	
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	if((rf_device_index > MAX_RF_ZONE_NUMBER ) || (rf_device_index == 0) ){
		ALARM_TRACE_ERROR("unsupported rf zone index: %d \n", rf_device_index);
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	ALARM_TRACE_INFO("START: stop_rf_device_action rf_device_index = %d \n", rf_device_index);
	rf_last_action_str.action_flag = 1; 
	rf_last_action_str.action_rf_type = PENDANT;
	rf_last_action_str.action_index = rf_device_index;
	rf_last_action_str.rf_action = CRED_ADD_RF_DEVICES;
	Stop_Learning_Device();
	ALARM_TRACE_INFO("END: stop_rf_device_action rf_device_index = %d \n", rf_device_index);
	return err;
}
/**
 * \fn CRED_MW_Errors_t reset_rf_zone_setting(void)
 * \brief load RF zone global structure from NAND flash.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t reset_rf_zone_setting(uint8_t zone_index)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	FILE *fp_rf_zone=NULL;
	int writebyte=0;
	
	if((zone_index > MAX_RF_ZONE_NUMBER ) || (zone_index == 0 ))
	{
		ALARM_TRACE_ERROR("unsupported rf zone index: %d \n", zone_index);
		return CRED_MW_ERROR_BAD_PARAMETER;
	}

	((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_index = zone_index;
	((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_id = 0;
	((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->add_status = CRED_RF_DEVICES_NOT_USED;
	((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_type = CRED_RF_ZONE_ALARM;
	((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_area = CRED_RF_ZONE_AREA1;
	((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_area_status = 0; /* TBV*/
	((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_attrebute = CRED_ZONE_ATTRIBUTES_NOT_USED;
	((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_output = 0;/* TBV*/
	((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_partition = CRED_ZONE_PARTITION_ALL; /* ALL */
	((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_partition_status = CRED_ARM_MODE_DISARM;
	((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_alarm_status = CRED_ZONE_ALARM_STATUS_NO_ALARM;
	((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_arm_status = CRED_ZONE_ARM_STATUS_DISARMED;
	((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_status = CRED_ZONE_STATUS_NOT_USED;
	((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_entry_delay = DEFAULT_ENTRY_EXIT_TIME;
	((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_exit_delay = DEFAULT_ENTRY_EXIT_TIME;
	((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_extra_entry_delay = 0;
	((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_delay = 0;/* TBV*/
	((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_logic_type = CRED_ZONE_LOGIC_NOT_USED; /*!<  ogic area : Or / And */
	((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_logic_area = CRED_RF_ZONE_AREA_NOT_USED; /*!<  And logic area : (A8A7A6A5A4A3A2A1: 0/1)*/
	((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_forced_arm_level = CRED_FORCED_ARM_LEVEL_NOT_USED;
	memset(((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_description,0,CRED_RF_DEVICE_DESCRIPTION_STR);
	sprintf(((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_description,"Zone %02d",zone_index);
	((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->rf_zone_detector_ptr = NULL;
	((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->rf_zone_pendant_ptr = NULL;

	/*Open RF Zone file:*/
	fp_rf_zone = fopen(CRED_RF_ZONE_FILE_PATH, "r+");
	fseek(fp_rf_zone, (sizeof(alarm_service_rf_zone_t)*zone_index), SEEK_SET);
	writebyte = fwrite(((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index)), (sizeof(alarm_service_rf_zone_t)), 1, fp_rf_zone);
	if(fp_rf_zone)
    fclose(fp_rf_zone);
	alarm_delete_zone_from_area(zone_index);
	return err;
}

/**
 * \fn CRED_MW_Errors_t reset_rf_output_setting(uint8_t output_index)
 * \brief load RF zone global structure from NAND flash.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t reset_rf_output_setting(uint8_t output_index)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	
	if((output_index > MAX_RF_OUTPUT_NUMBER ) || (output_index == 0 ))
	{
		ALARM_TRACE_ERROR("unsupported rf output index: %d \n", output_index);
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	((alarm_service_rf_output_t *)(rf_output_global_ptr + output_index))->output_index = output_index;
	((alarm_service_rf_output_t *)(rf_output_global_ptr + output_index))->output_id = 0;
	((alarm_service_rf_output_t *)(rf_output_global_ptr + output_index))->add_status = CRED_RF_DEVICES_NOT_USED;
	((alarm_service_rf_output_t *)(rf_output_global_ptr + output_index))->output_type = CRED_OUTPUT_NOT_USED;
	((alarm_service_rf_output_t *)(rf_output_global_ptr + output_index))->output_time = 0;
	((alarm_service_rf_output_t *)(rf_output_global_ptr + output_index))->output_area = 1;
	((alarm_service_rf_output_t *)(rf_output_global_ptr + output_index))->output_area_status = 0;
	((alarm_service_rf_output_t *)(rf_output_global_ptr + output_index))->output_attrebute = 0;
		
	((alarm_service_rf_output_t *)(rf_output_global_ptr + output_index))->output_partition = 4; /* ALL */
	((alarm_service_rf_output_t *)(rf_output_global_ptr + output_index))->output_partition_status = 0;
	((alarm_service_rf_output_t *)(rf_output_global_ptr + output_index))->output_alarm_status = 0;
			
	((alarm_service_rf_output_t *)(rf_output_global_ptr + output_index))->output_activate_status = 0;
	((alarm_service_rf_output_t *)(rf_output_global_ptr + output_index))->output_delay = 0;
	memset(((alarm_service_rf_output_t *)(rf_output_global_ptr + output_index))->output_description,0,CRED_RF_DEVICE_DESCRIPTION_STR);
	((alarm_service_rf_output_t *)(rf_output_global_ptr + output_index))->rf_output_ptr = NULL;	

	return err;
}

/**
 * \fn CRED_MW_Errors_t reset_rf_output_setting(uint8_t output_index)
 * \brief load RF zone global structure from NAND flash.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t reset_rf_reemote_setting(uint8_t remote_index)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	
	if((remote_index > MAX_RF_REMOTE_NUMBER) || (remote_index == 0 ))
	{
		ALARM_TRACE_ERROR("unsupported rf remote index: %d \n", remote_index);
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	
	((alarm_service_rf_remote_t *)(rf_remote_global_ptr + remote_index))->remote_index = remote_index;
	((alarm_service_rf_remote_t *)(rf_remote_global_ptr + remote_index))->remote_id = 0;
	((alarm_service_rf_remote_t *)(rf_remote_global_ptr + remote_index))->add_status = CRED_RF_DEVICES_NOT_USED;
	((alarm_service_rf_remote_t *)(rf_remote_global_ptr + remote_index))->remote_type = 0;
	((alarm_service_rf_remote_t *)(rf_remote_global_ptr + remote_index))->remote_area = 1;
	((alarm_service_rf_remote_t *)(rf_remote_global_ptr + remote_index))->remote_area_status = 0;
	((alarm_service_rf_remote_t *)(rf_remote_global_ptr + remote_index))->remote_partition = 4; /* ALL */
	((alarm_service_rf_remote_t *)(rf_remote_global_ptr + remote_index))->remote_partition_status = 0;
	((alarm_service_rf_remote_t *)(rf_remote_global_ptr + remote_index))->remote_alarm_status = 0;
	((alarm_service_rf_remote_t *)(rf_remote_global_ptr + remote_index))->remote_activate_status = 0;
	((alarm_service_rf_remote_t *)(rf_remote_global_ptr + remote_index))->rf_remote_ptr = NULL;
	
	return err;
}

/**
 * \fn CRED_MW_Errors_t load_rf_zone_information(void)
 * \brief load RF zone global structure from NAND flash.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t load_rf_zone_information(void)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	int i;
	FILE *fp_rf_zone=NULL; 
	/*Open RF Zone file:*/
	fp_rf_zone = fopen(CRED_RF_ZONE_FILE_PATH, "r");
	if(fp_rf_zone == NULL)
	{
		/*File Does Not Exit => Create the File*/
		fp_rf_zone = fopen(CRED_RF_ZONE_FILE_PATH, "w+");
		
		for(i=0;i<=MAX_RF_ZONE_NUMBER;i++)
		{
			((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_index = i;
			((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_id = 0;
			((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->add_status = CRED_RF_DEVICES_NOT_USED;
			((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_type = CRED_RF_ZONE_ALARM;
			((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_area = CRED_RF_ZONE_AREA1;
			((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_area_status = 0; /* TBV*/
			((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_attrebute = CRED_ZONE_ATTRIBUTES_NOT_USED;
			((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_output = 0;/* TBV*/
			((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_partition = CRED_ZONE_PARTITION_ALL; /* ALL */
			((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_partition_status = CRED_ARM_MODE_DISARM;
			((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_alarm_status = CRED_ZONE_ALARM_STATUS_NO_ALARM;
			((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_arm_status = CRED_ZONE_ARM_STATUS_DISARMED;
			((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_status = CRED_ZONE_STATUS_NOT_USED;
			((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_entry_delay = DEFAULT_ENTRY_EXIT_TIME;
			((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_exit_delay = DEFAULT_ENTRY_EXIT_TIME;
			((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_extra_entry_delay = 0;
			((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_delay = 0;/* TBV*/
			((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_logic_type = CRED_ZONE_LOGIC_NOT_USED; /*!<  ogic area : Or / And */
			((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_logic_area = CRED_RF_ZONE_AREA_NOT_USED; /*!<  And logic area : (A8A7A6A5A4A3A2A1: 0/1)*/
			((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_forced_arm_level = CRED_FORCED_ARM_LEVEL_NOT_USED;
			memset(((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_description,0,CRED_RF_DEVICE_DESCRIPTION_STR);
			sprintf(((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_description,"Zone %02d",i);
			((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->rf_zone_detector_ptr = NULL;
			((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->rf_zone_pendant_ptr = NULL;
		}
		fwrite(rf_zone_global_ptr, (sizeof(alarm_service_rf_zone_t)*(MAX_RF_ZONE_NUMBER+1)), 1, fp_rf_zone);
		ALARM_TRACE_INFO("Create RF Zone File for saving RF Zone Status \n ");
	}
	else
	{
		/*File Exist Read Old Information*/
		fread(rf_zone_global_ptr, (sizeof(alarm_service_rf_zone_t)*(MAX_RF_ZONE_NUMBER+1)), 1, fp_rf_zone);
		ALARM_TRACE_INFO("RF Zone Read Saved Detectors Information \n ");
	}
	/*Close RF Crow file*/
	fclose(fp_rf_zone);
	return err;
}
/**
 * \fn CRED_MW_Errors_t load_rf_output_information(void)
 * \brief load RF zone global structure from NAND flash.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t load_rf_output_information(void)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	int i;
	FILE *fp_rf_output=NULL; 
	/*Open RF output file:*/
	fp_rf_output = fopen(CRED_RF_OUTPUT_FILE_PATH, "r");
	if(fp_rf_output == NULL)
	{
		/*File Does Not Exit => Create the File*/
		fp_rf_output = fopen(CRED_RF_OUTPUT_FILE_PATH, "w+");
		
		for(i=0;i<=MAX_RF_OUTPUT_NUMBER;i++)
		{
			((alarm_service_rf_output_t *)(rf_output_global_ptr + i))->output_index = i;
			((alarm_service_rf_output_t *)(rf_output_global_ptr + i))->output_id = 0;
			((alarm_service_rf_output_t *)(rf_output_global_ptr + i))->add_status = CRED_RF_DEVICES_NOT_USED;
			((alarm_service_rf_output_t *)(rf_output_global_ptr + i))->output_type = CRED_OUTPUT_NOT_USED;
			((alarm_service_rf_output_t *)(rf_output_global_ptr + i))->output_time = 0;
			((alarm_service_rf_output_t *)(rf_output_global_ptr + i))->output_area = 1;
			((alarm_service_rf_output_t *)(rf_output_global_ptr + i))->output_area_status = 0;
			((alarm_service_rf_output_t *)(rf_output_global_ptr + i))->output_attrebute = 0;
		
			((alarm_service_rf_output_t *)(rf_output_global_ptr + i))->output_partition = 4; /* ALL */
			((alarm_service_rf_output_t *)(rf_output_global_ptr + i))->output_partition_status = 0;
			((alarm_service_rf_output_t *)(rf_output_global_ptr + i))->output_alarm_status = 0;
			
			((alarm_service_rf_output_t *)(rf_output_global_ptr + i))->output_activate_status = 0;
			((alarm_service_rf_output_t *)(rf_output_global_ptr + i))->output_delay = 0;
			memset(((alarm_service_rf_output_t *)(rf_output_global_ptr + i))->output_description,0,CRED_RF_DEVICE_DESCRIPTION_STR);
			((alarm_service_rf_output_t *)(rf_output_global_ptr + i))->rf_output_ptr = NULL;
			
		}
		output_index_global = 0;
		fwrite(rf_output_global_ptr, (sizeof(alarm_service_rf_output_t)*(MAX_RF_OUTPUT_NUMBER+1)), 1, fp_rf_output);
		ALARM_TRACE_INFO("Create RF output File for saving RF output Status \n ");
	}
	else
	{
		/*File Exist Read Old Information*/
		fread(rf_output_global_ptr, (sizeof(alarm_service_rf_output_t)*(MAX_RF_OUTPUT_NUMBER+1)), 1, fp_rf_output);
		for(i=1;i<=MAX_RF_OUTPUT_NUMBER;i++)
		{
			if(((alarm_service_rf_output_t *)(rf_output_global_ptr + i))->add_status  != CRED_RF_DEVICES_NOT_USED)
			{
				output_index_global = i;
				i=MAX_RF_OUTPUT_NUMBER+1;
			}
		}
		ALARM_TRACE_INFO("RF output Read Saved  Information \n ");
	}
	/*Close RF Crow file*/
	fclose(fp_rf_output);
	return err;
}
/**
 * \fn CRED_MW_Errors_t load_rf_remote_information(void)
 * \brief load RF remote global structure from NAND flash.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t load_rf_remote_information(void)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	int i;
	FILE *fp_rf_remote=NULL; 
	/*Open RF Remote file:*/
	fp_rf_remote = fopen(CRED_RF_REMOTE_FILE_PATH, "r");
	if(fp_rf_remote == NULL)
	{
		/*File Does Not Exit => Create the File*/
		fp_rf_remote = fopen(CRED_RF_REMOTE_FILE_PATH, "w+");
		
		for(i=0;i<=MAX_RF_REMOTE_NUMBER;i++)
		{
			((alarm_service_rf_remote_t *)(rf_remote_global_ptr + i))->remote_index = i;
			((alarm_service_rf_remote_t *)(rf_remote_global_ptr + i))->remote_id = 0;
			((alarm_service_rf_remote_t *)(rf_remote_global_ptr + i))->add_status = CRED_RF_DEVICES_NOT_USED;
			((alarm_service_rf_remote_t *)(rf_remote_global_ptr + i))->remote_type = 0;
			((alarm_service_rf_remote_t *)(rf_remote_global_ptr + i))->remote_area = 1;
			((alarm_service_rf_remote_t *)(rf_remote_global_ptr + i))->remote_area_status = 0;
			((alarm_service_rf_remote_t *)(rf_remote_global_ptr + i))->remote_partition = 4; /* ALL */
			((alarm_service_rf_remote_t *)(rf_remote_global_ptr + i))->remote_partition_status = 0;
			((alarm_service_rf_remote_t *)(rf_remote_global_ptr + i))->remote_alarm_status = 0;
			((alarm_service_rf_remote_t *)(rf_remote_global_ptr + i))->remote_activate_status = 0;
			((alarm_service_rf_remote_t *)(rf_remote_global_ptr + i))->rf_remote_ptr = NULL;
		}
		
		fwrite(rf_remote_global_ptr, (sizeof(alarm_service_rf_remote_t)*(MAX_RF_REMOTE_NUMBER+1)), 1, fp_rf_remote);
		ALARM_TRACE_INFO("Create RF Remote File for saving RF Remote Status \n ");
	}
	else
	{
		/*File Exist Read Old Information*/
		fread(rf_remote_global_ptr, (sizeof(alarm_service_rf_remote_t)*(MAX_RF_REMOTE_NUMBER+1)), 1, fp_rf_remote);
		ALARM_TRACE_INFO("RF Remote Read Saved Remote Information \n ");
	}
	/*Close RF Crow file*/
	fclose(fp_rf_remote);
	return err;
}
/**
 * \fn CRED_MW_Errors_t save_rf_zone_information(uint8_t zone_index)
 * \brief save RF zone  structure to NAND flash.
 *
 * \param zone_index: zone index.
 * \return No parameter.
 */
CRED_MW_Errors_t save_rf_zone_information(uint8_t zone_index)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	int writebyte = 0;
	FILE *fp_rf_zone=NULL;
	
	if((zone_index > MAX_RF_ZONE_NUMBER ) || (zone_index == 0 )){
		ALARM_TRACE_ERROR("unsupported rf zone index: %d \n", zone_index);
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	
	/*Open RF Zone file:*/
	((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_partition |= CRED_ZONE_PARTITION_ALL;

	ALARM_TRACE_INFO("Zone index 					= %d\n", ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_index);
	ALARM_TRACE_INFO("zone_id 						= %d\n", ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_id);
	ALARM_TRACE_INFO("add_status 					= 0x%x\n", ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->add_status);
	ALARM_TRACE_INFO("zone_type 					= %d\n", ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_type);
	ALARM_TRACE_INFO("zone_area 					= 0x%x\n", ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_area);
	ALARM_TRACE_INFO("zone_area_status				= 0x%x\n", ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_area_status);
	ALARM_TRACE_INFO("zone_attrebute				= 0x%x\n", ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_attrebute);
	ALARM_TRACE_INFO("zone_output 					= 0x%x\n", ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_output);
	ALARM_TRACE_INFO("zone_partition				= %d\n", ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_partition);
	ALARM_TRACE_INFO("zone_partition_status			= %d\n", ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_partition_status);
	ALARM_TRACE_INFO("zone_alarm_status 	 		= %d\n", ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_alarm_status);
	ALARM_TRACE_INFO("zone_arm_status 				= %d\n", ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_arm_status);
	ALARM_TRACE_INFO("zone_status 				    = %d\n", ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_status);
	ALARM_TRACE_INFO("entry_delay			 		= %d\n", ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_entry_delay);
	ALARM_TRACE_INFO("exit_delay			 		= %d\n", ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_exit_delay);
	ALARM_TRACE_INFO("zone_delay			 		= %d\n", ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_delay);

	fp_rf_zone = fopen(CRED_RF_ZONE_FILE_PATH, "r+");
	fseek(fp_rf_zone, (sizeof(alarm_service_rf_zone_t)*zone_index), SEEK_SET);
	writebyte = fwrite(((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index)), (sizeof(alarm_service_rf_zone_t)), 1, fp_rf_zone);
	if(fp_rf_zone)
    fclose(fp_rf_zone);
    
    alarm_add_zone_to_area(zone_index,((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_area);

	return err;
}

/**
 * \fn CRED_MW_Errors_t alarm_rf_zone_added(uint8_t zone_index)
 * \brief save RF zone  structure to NAND flash.
 *
 * \param zone_index: zone index.
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_rf_zone_added(uint8_t zone_index)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	int writebyte = 0;
	FILE *fp_rf_zone=NULL;

	if((zone_index > MAX_RF_ZONE_NUMBER ) || (zone_index == 0 )){
		ALARM_TRACE_ERROR("unsupported rf zone index: %d \n", zone_index);
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	/*Open RF Zone file:*/
	((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_partition |= CRED_ZONE_PARTITION_ALL;
	((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_status |= CRED_ZONE_STATUS_INCLUDED;
	((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_status |= CRED_ZONE_STATUS_ACTIVATED;
	((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_status |= CRED_ZONE_STATUS_INHIBITED_NO;

	ALARM_TRACE_INFO("Zone index 					= %d\n", ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_index);
	ALARM_TRACE_INFO("zone_id 						= %d\n", ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_id);
	ALARM_TRACE_INFO("add_status 					= 0x%x\n", ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->add_status);
	ALARM_TRACE_INFO("zone_type 					= %d\n", ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_type);
	ALARM_TRACE_INFO("zone_area 					= 0x%x\n", ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_area);
	ALARM_TRACE_INFO("zone_area_status				= 0x%x\n", ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_area_status);
	ALARM_TRACE_INFO("zone_attrebute				= 0x%x\n", ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_attrebute);
	ALARM_TRACE_INFO("zone_output 					= 0x%x\n", ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_output);
	ALARM_TRACE_INFO("zone_partition				= %d\n", ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_partition);
	ALARM_TRACE_INFO("zone_partition_status			= %d\n", ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_partition_status);
	ALARM_TRACE_INFO("zone_alarm_status 	 		= %d\n", ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_alarm_status);
	ALARM_TRACE_INFO("zone_arm_status 				= %d\n", ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_arm_status);
	ALARM_TRACE_INFO("zone_status 				    = %d\n", ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_status);
	ALARM_TRACE_INFO("entry_delay			 		= %d\n", ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_entry_delay);
	ALARM_TRACE_INFO("exit_delay			 		= %d\n", ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_exit_delay);
	ALARM_TRACE_INFO("zone_delay			 		= %d\n", ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_delay);

	fp_rf_zone = fopen(CRED_RF_ZONE_FILE_PATH, "r+");
	fseek(fp_rf_zone, (sizeof(alarm_service_rf_zone_t)*zone_index), SEEK_SET);
	writebyte = fwrite(((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index)), (sizeof(alarm_service_rf_zone_t)), 1, fp_rf_zone);
	if(fp_rf_zone)
    fclose(fp_rf_zone);

    alarm_add_zone_to_area(zone_index,((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index))->zone_area);

	return err;
}

/**
 * \fn CRED_MW_Errors_t get_rf_zone_information(uint8_t zone_index)
 * \brief get RF zone  informations.
 *
 * \param zone_index: zone index.
 * \return No parameter.
 */

CRED_MW_Errors_t get_rf_zone_information(uint8_t rf_zone_index, uint32_t *rf_zone_ptr)
{
	
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	
	alarm_service_rf_zone_t *rf_zone_tmp_ptr=NULL;
	ALARM_TRACE_INFO("START: get_rf_zone_information rf_zone_index = %d rf_zone_ptr =0x%x\n", rf_zone_index,rf_zone_ptr);
	if((rf_zone_index > MAX_RF_ZONE_NUMBER ) || (rf_zone_index == 0 )){
		ALARM_TRACE_ERROR("unsupported rf zone index: %d \n", rf_zone_index);
		*rf_zone_ptr = NULL;
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	
	rf_zone_tmp_ptr=((alarm_service_rf_zone_t *)(rf_zone_global_ptr + rf_zone_index));
	
	ALARM_TRACE_INFO("Zone index 					= %d\n", ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + rf_zone_index))->zone_index);
	ALARM_TRACE_INFO("zone_id 						= %d\n", ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + rf_zone_index))->zone_id);
	ALARM_TRACE_INFO("add_status 					= 0x%x\n", ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + rf_zone_index))->add_status);
	ALARM_TRACE_INFO("zone_type 					= %d\n", ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + rf_zone_index))->zone_type);
	ALARM_TRACE_INFO("zone_area 					= 0x%x\n", ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + rf_zone_index))->zone_area);
	ALARM_TRACE_INFO("zone_area_status				= 0x%x\n", ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + rf_zone_index))->zone_area_status);
	ALARM_TRACE_INFO("zone_attrebute				= 0x%x\n", ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + rf_zone_index))->zone_attrebute);
	ALARM_TRACE_INFO("zone_output 					= 0x%x\n", ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + rf_zone_index))->zone_output);
	ALARM_TRACE_INFO("zone_partition				= %d\n", ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + rf_zone_index))->zone_partition);
	ALARM_TRACE_INFO("zone_partition_status			= %d\n", ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + rf_zone_index))->zone_partition_status);
	ALARM_TRACE_INFO("zone_alarm_status 	 		= %d\n", ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + rf_zone_index))->zone_alarm_status);
	ALARM_TRACE_INFO("zone_arm_status 				= %d\n", ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + rf_zone_index))->zone_arm_status);
	ALARM_TRACE_INFO("zone_status 					= %d\n", ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + rf_zone_index))->zone_status);
	ALARM_TRACE_INFO("entry_delay			 		= %d\n", ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + rf_zone_index))->zone_entry_delay);
	ALARM_TRACE_INFO("exit_delay			 		= %d\n", ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + rf_zone_index))->zone_exit_delay);
	ALARM_TRACE_INFO("zone_delay			 		= %d\n", ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + rf_zone_index))->zone_delay);
	
	*rf_zone_ptr = rf_zone_tmp_ptr;
	return err;
}

/**
 * \fn CRED_MW_Errors_t save_rf_output_information(uint8_t output_index)
 * \brief save RF output  structure to NAND flash.
 *
 * \param output_index: output index.
 * \return No parameter.
 */
CRED_MW_Errors_t save_rf_output_information(uint8_t output_index)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	int writebyte = 0;
	FILE *fp_rf_output=NULL;

	/*Open RF output file:*/
	fp_rf_output = fopen(CRED_RF_OUTPUT_FILE_PATH, "r+");
	
	fseek(fp_rf_output, (sizeof(alarm_service_rf_output_t)*output_index), SEEK_SET);
	
	writebyte = fwrite(((alarm_service_rf_output_t *)(rf_output_global_ptr + output_index)), (sizeof(alarm_service_rf_output_t)), 1, fp_rf_output);
	
	if(fp_rf_output)
    fclose(fp_rf_output);
    
	return err;
}
/**
 * \fn CRED_MW_Errors_t save_rf_remote_information(uint8_t remote_index)
 * \brief save RF Remote  structure to NAND flash.
 *
 * \param remote_index: remote index.
 * \return No parameter.
 */
CRED_MW_Errors_t save_rf_remote_information(uint8_t remote_index)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	int writebyte = 0;
	FILE *fp_rf_remote=NULL;

	/*Open RF Remote file:*/
	fp_rf_remote = fopen(CRED_RF_REMOTE_FILE_PATH, "r+");
	
	fseek(fp_rf_remote, (sizeof(alarm_service_rf_remote_t)*remote_index), SEEK_SET);
	
	writebyte = fwrite(((alarm_service_rf_remote_t *)(rf_remote_global_ptr + remote_index)), (sizeof(alarm_service_rf_remote_t)), 1, fp_rf_remote);
	
	if(fp_rf_remote)
    fclose(fp_rf_remote);
    
	return err;
}

/**
 * \fn CRED_MW_Errors_t load_user_code_information(void)
 * \brief load user global structure from NAND flash.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t load_user_code_information(void)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	int i;
	FILE *fp_user_code=NULL; 
	/*Open User Code file:*/
	fp_user_code = fopen(CRED_USER_FILE_PATH, "r");
	if(fp_user_code == NULL)
	{
		/*File Does Not Exit => Create the File*/
		fp_user_code = fopen(CRED_USER_FILE_PATH, "w+");
		
		for(i=0;i<=MAX_USER_CODE;i++)
		{
			((alarm_service_user_code_t *)(user_code_global_ptr + i))->user_index = i;
			if(i == 1){
				
				((alarm_service_user_code_t *)(user_code_global_ptr + i))->user_status = 1; /*!<  Only user 001 enabled by default*/
				
			}else
			{
				((alarm_service_user_code_t *)(user_code_global_ptr + i))->user_status = 0;
			}
			memset(((alarm_service_user_code_t *)(user_code_global_ptr + i))->user_code,0,MAX_CODE_LEN_STR); /*!< User description default USER INDEX */
			
			sprintf(((alarm_service_user_code_t *)(user_code_global_ptr + i))->user_code,"1111",0);
			((alarm_service_user_code_t *)(user_code_global_ptr + i))->user_profile_index = 1; /*!<  User profile : from 1 to 16 default master*/
			((alarm_service_user_code_t *)(user_code_global_ptr + i))->user_schedule = 1; /*!<  User schedule*/
			((alarm_service_user_code_t *)(user_code_global_ptr + i))->user_area = 1; /*!<  Default area1*/
			((alarm_service_user_code_t *)(user_code_global_ptr + i))->user_control_devices = 1; /*!<  Default device : control panel*/
			memset(((alarm_service_user_code_t *)(user_code_global_ptr + i))->user_description,0,CRED_USER_DESCRIPTION_STR); /*!< User description default USER INDEX */
			sprintf(((alarm_service_user_code_t *)(user_code_global_ptr + i))->user_description,"USER %03d",i);

		}
		
		fwrite(user_code_global_ptr, (sizeof(alarm_service_user_code_t)*(MAX_USER_CODE+1)), 1, fp_user_code);
		ALARM_TRACE_INFO("Create user code File for saving user code information \n ");
	}
	else
	{
		/*File Exist Read Old Information*/
		fread(user_code_global_ptr, (sizeof(alarm_service_user_code_t)*(MAX_USER_CODE+1)), 1, fp_user_code);
		ALARM_TRACE_INFO("Read User Code Saved  Information \n ");
	}
	/*Close User Code file*/
	fclose(fp_user_code);
	return err;
}
/**
 * \fn void alarm_factory_reset_user(void)
 * \brief Factory reset user.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_factory_reset_user(void)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	int i;
	FILE *fp_user_code=NULL;
	ALARM_TRACE_INFO("START: alarm_factory_reset_user \n"); 
	/*Open User Code file:*/
	fp_user_code = fopen(CRED_USER_FILE_PATH, "r+");
	if(fp_user_code == NULL)
	{
		/*File Does Not Exit => Create the File*/
		fp_user_code = fopen(CRED_USER_FILE_PATH, "w+");
	}
	for(i=1;i<=MAX_USER_CODE;i++)
	{
			((alarm_service_user_code_t *)(user_code_global_ptr + i))->user_index = i;
			if(i == 1){
				
				((alarm_service_user_code_t *)(user_code_global_ptr + i))->user_status = 1; /*!<  Only user 001 enabled by default*/
				
			}else
			{
				((alarm_service_user_code_t *)(user_code_global_ptr + i))->user_status = 0;
			}
			memset(((alarm_service_user_code_t *)(user_code_global_ptr + i))->user_code,0,MAX_CODE_LEN_STR); /*!< User description default USER INDEX */
			
			sprintf(((alarm_service_user_code_t *)(user_code_global_ptr + i))->user_code,"1111",0);
			((alarm_service_user_code_t *)(user_code_global_ptr + i))->user_profile_index = 1; /*!<  User profile : from 1 to 16 default master*/
			((alarm_service_user_code_t *)(user_code_global_ptr + i))->user_schedule = 1; /*!<  User schedule*/
			((alarm_service_user_code_t *)(user_code_global_ptr + i))->user_area = 1; /*!<  Default area1*/
			((alarm_service_user_code_t *)(user_code_global_ptr + i))->user_control_devices = 1; /*!<  Default device : control panel*/
			memset(((alarm_service_user_code_t *)(user_code_global_ptr + i))->user_description,0,CRED_USER_DESCRIPTION_STR); /*!< User description default USER INDEX */
			sprintf(((alarm_service_user_code_t *)(user_code_global_ptr + i))->user_description,"USER %03d",i);

	}
		
	fwrite(user_code_global_ptr, (sizeof(alarm_service_user_code_t)*(MAX_USER_CODE+1)), 1, fp_user_code);
	
	/*Close User Code file*/
	fclose(fp_user_code);
	ALARM_TRACE_INFO("END: alarm_factory_reset_user \n"); 
	return err;
}

/**
 * \fn CRED_MW_Errors_t check_user_code(uint32_t user_code,uint32_t *user_code_index, uint32_t *user_code_ptr)
 * \brief get RF zone  informations.
 *
 * \param zone_index: zone index.
 * \return No parameter.
 */

CRED_MW_Errors_t check_user_code(char *user_code,uint32_t *user_code_index, uint32_t *user_code_ptr)
{
	
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	uint8_t user_flag = 0;
	
	int i;
	for(i=1;i<=MAX_USER_CODE;i++)
	{
			if(strcmp(((alarm_service_user_code_t *)(user_code_global_ptr + i))->user_code,user_code) == 0)
			//if(((alarm_service_user_code_t *)(user_code_global_ptr + i))->user_code == user_code)
			{
				
				if(((alarm_service_user_code_t *)(user_code_global_ptr + i))->user_status == 1)
				{
					*user_code_index = i;
					*user_code_ptr = ((alarm_service_user_code_t *)(user_code_global_ptr + i));
					ALARM_TRACE_INFO("USER CODE %s entred : OK index %d \n", user_code,i);
					user_flag = 1;
					ALARM_TRACE_INFO("user_index 					= %d\n", ((alarm_service_user_code_t *)(user_code_global_ptr + i))->user_index);
					ALARM_TRACE_INFO("user_status 					= %d\n", ((alarm_service_user_code_t *)(user_code_global_ptr + i))->user_status);
					//ALARM_TRACE_INFO("user_code 					= %d\n", ((alarm_service_user_code_t *)(user_code_global_ptr + i))->user_code);
					ALARM_TRACE_INFO("user_code 					= %s\n", ((alarm_service_user_code_t *)(user_code_global_ptr + i))->user_code);
					ALARM_TRACE_INFO("user_profile_index 			= %d\n", ((alarm_service_user_code_t *)(user_code_global_ptr + i))->user_profile_index);
					ALARM_TRACE_INFO("user_schedule 				= %d\n", ((alarm_service_user_code_t *)(user_code_global_ptr + i))->user_schedule);
					ALARM_TRACE_INFO("user_area						= 0x%x\n", ((alarm_service_user_code_t *)(user_code_global_ptr + i))->user_area);
					ALARM_TRACE_INFO("user_control_devices			= 0x%x\n", ((alarm_service_user_code_t *)(user_code_global_ptr + i))->user_control_devices);
					ALARM_TRACE_INFO("user_description				= %s\n", ((alarm_service_user_code_t *)(user_code_global_ptr + i))->user_description);
					
					memcpy(&(current_user_code_str.user_index),((alarm_service_user_code_t *)(user_code_global_ptr + i)),(sizeof(alarm_service_user_code_t)));
					
				}else
				{
					ALARM_TRACE_INFO("USER CODE %s entred : OK index %d !!!! NOT ACTIVATED \n", user_code,i);
					user_flag = 2;
				}
				
				i = MAX_USER_CODE + 1;
			}
			if(user_flag !=0 )
			{
				break;
			}
	}
	if((user_flag == 0) || (user_flag == 2))
	{
		*user_code_index = 0;
		*user_code_ptr = NULL;
		ALARM_TRACE_INFO("WRONG !!!!! USER CODE %s entred \n", user_code);
		/* ADD NOTIFY */
	}
	return err;
}

/**
 * \fn CRED_MW_Errors_t alarm_save_user_info(uint32_t *user_code_app_index, alarm_service_user_code_t *user_code_ptr)
 * \brief save user code informations.
 *
 * \param uint32_t *user_code_app_index, alarm_service_user_code_t *user_code_ptr
 * \return No parameter.
 */

CRED_MW_Errors_t alarm_save_user_info(/*char *user_code,*/uint32_t *user_code_app_index, alarm_service_user_code_t *user_code_ptr)
{
	
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	int user_code_index,user_code_len;
	FILE *fp_user_code=NULL;
	int writebyte = 0;

	user_code_len  = strlen(user_code_ptr->user_code);

	ALARM_TRACE_INFO("USER CODE Len : %d \n", user_code_len);

	if(user_code_len > MAX_CODE_LEN)
	{
		user_code_len = MAX_CODE_LEN;
	}

	user_code_index = *user_code_app_index;
	if((user_code_index > MAX_USER_CODE) || (user_code_index < 1) || (user_code_len < MIN_CODE_LEN ))
	{
		ALARM_TRACE_ERROR("unsupported user code index: %d or code len\n", user_code_index);
		*user_code_app_index = 0;
		return err;
	}

	memcpy(((alarm_service_user_code_t *)(user_code_global_ptr + user_code_index)),user_code_ptr,(sizeof(alarm_service_user_code_t)));

	/*Open RF user file:*/
	fp_user_code = fopen(CRED_USER_FILE_PATH, "r+");

	fseek(fp_user_code, (sizeof(alarm_service_user_code_t)*user_code_index), SEEK_SET);

	writebyte = fwrite(((alarm_service_user_code_t *)(user_code_global_ptr + user_code_index)), (sizeof(alarm_service_user_code_t)), 1, fp_user_code);

	if(fp_user_code)
	fclose(fp_user_code);

	return err;
}

/**
 * \fn CRED_MW_Errors_t alarm_get_user_info(uint32_t *user_code_app_index, alarm_service_user_code_t *user_code_ptr)
 * \brief get user code  informations.
 *
 * \param uint32_t *user_code_app_index, alarm_service_user_code_t *user_code_ptr
 * \return No parameter.
 */

CRED_MW_Errors_t alarm_get_user_info(uint32_t *user_code_app_index, alarm_service_user_code_t *user_code_ptr)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	int user_code_index;

	user_code_index = *user_code_app_index;
	if((user_code_index > MAX_USER_CODE) || (user_code_index < 1))
	{
		ALARM_TRACE_ERROR("unsupported user code index: %d or code len\n", user_code_index);
		*user_code_app_index = 0;
		return err;
	}
	if(user_code_ptr == NULL)
	{
		ALARM_TRACE_ERROR("unsupported user pointer: 0x%x\n", user_code_ptr);
		*user_code_app_index = 0;
		return err;
	}

	memcpy(user_code_ptr,((alarm_service_user_code_t *)(user_code_global_ptr + user_code_index)),(sizeof(alarm_service_user_code_t)));

	return err;
}

/**
 * \fn CRED_MW_Errors_t alarm_save_installer_info(uint32_t *installer_code_app_index, alarm_service_installer_code_t *installer_code_ptr)
 * \brief save installer informations.
 *
 * \param uint32_t *installer_code_app_index, alarm_service_installer_code_t *installer_code_ptr
 * \return No parameter.
 */

CRED_MW_Errors_t alarm_save_installer_info(/*char *installer_code,*/uint32_t *installer_code_app_index, alarm_service_installer_code_t *installer_code_ptr)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	int installer_code_index,installer_code_len;
	FILE *fp_installer_code=NULL;
	int writebyte = 0;

	installer_code_len  = strlen(installer_code_ptr->installer_code);

	ALARM_TRACE_INFO("installer CODE Len : %d \n", installer_code_len);

	if(installer_code_len > MAX_CODE_LEN)
	{
		installer_code_len = MAX_CODE_LEN;
	}

	installer_code_index = *installer_code_app_index;
	if((installer_code_index > MAX_INSTALLER_CODE) || (installer_code_index < 1) || (installer_code_len < MIN_CODE_LEN ))
	{
		ALARM_TRACE_ERROR("unsupported installer code index: %d or code len\n",installer_code_index);
		*installer_code_app_index = 0;
		return err;
	}

	memcpy(((alarm_service_installer_code_t *)(installer_code_global_ptr + installer_code_index)),installer_code_ptr,(sizeof(alarm_service_installer_code_t)));

	/*Open RF installer file:*/
	fp_installer_code = fopen(CRED_INSTALLER_FILE_PATH, "r+");

	fseek(fp_installer_code, (sizeof(alarm_service_installer_code_t)*installer_code_index), SEEK_SET);

	writebyte = fwrite(((alarm_service_installer_code_t *)(installer_code_global_ptr + installer_code_index)), (sizeof(alarm_service_installer_code_t)), 1, fp_installer_code);

	if(fp_installer_code)
	fclose(fp_installer_code);

	return err;
}
/**
 * \fn CRED_MW_Errors_t alarm_save_installer_info(uint32_t *installer_code_app_index, alarm_service_installer_code_t *installer_code_ptr)
 * \brief save installer informations.
 *
 * \param uint32_t *installer_code_app_index, alarm_service_installer_code_t *installer_code_ptr
 * \return No parameter.
 */

CRED_MW_Errors_t alarm_get_installer_info(uint32_t *installer_code_app_index, alarm_service_installer_code_t *installer_code_ptr)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	int installer_code_index;

	installer_code_index = *installer_code_app_index;

	if((installer_code_index > MAX_INSTALLER_CODE) || (installer_code_index < 1))
	{
		ALARM_TRACE_ERROR("unsupported installer code index: %d or code len\n",installer_code_index);
		*installer_code_app_index = 0;
		return err;
	}
	if(installer_code_ptr == NULL)
	{
		ALARM_TRACE_ERROR("unsupported installer pointer: 0x%x\n",installer_code_ptr);
		*installer_code_app_index = 0;
		return err;
	}
	memcpy(installer_code_ptr,((alarm_service_installer_code_t *)(installer_code_global_ptr + installer_code_index)),(sizeof(alarm_service_installer_code_t)));
	return err;
}
/**
 * \fn CRED_MW_Errors_t change_user_code(uint32_t user_code,uint8_t user_code_index)
 * \brief get RF zone  informations.
 *
 * \param zone_index: zone index.
 * \return No parameter.
 */

CRED_MW_Errors_t change_user_code(char *user_code,uint32_t *user_code_app_index)
{
	
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	FILE *fp_user_code=NULL;
	int writebyte = 0;
	int i,user_code_index;
	int user_code_len = 0;
	uint8_t user_flag = 0;
	user_code_len  = strlen(user_code);
	
	ALARM_TRACE_INFO("USER CODE Len : %d \n", user_code_len);
	
	if(user_code_len > MAX_CODE_LEN)
	{
		user_code_len = MAX_CODE_LEN;
	}
	
	user_code_index = *user_code_app_index;
	if((user_code_index > MAX_USER_CODE) || (user_code_index < 1) || (user_code_len < MIN_CODE_LEN ))
	{
		ALARM_TRACE_ERROR("unsupported user code %s or index: %d \n", user_code, user_code_index);
		*user_code_app_index = 0;
		user_flag = 1;
		return err;
	}
	
	for(i=1;i<=MAX_USER_CODE;i++)
	{
			//if((((alarm_service_user_code_t *)(user_code_global_ptr + i))->user_code == user_code) && (((alarm_service_user_code_t *)(user_code_global_ptr + i))->user_status == 1))
			if((strcmp(((alarm_service_user_code_t *)(user_code_global_ptr + i))->user_code,user_code) == 0) && (((alarm_service_user_code_t *)(user_code_global_ptr + i))->user_status == 1))
			{
				
				ALARM_TRACE_INFO("USER CODE %s entred : Used\n", user_code);
				i = MAX_USER_CODE + 1;
				*user_code_app_index = 0;
				user_flag = 1;
			}
			if(user_flag == 1)
			{
				break;
			}
	}
	if(user_flag == 0){
		 user_code_index = *user_code_app_index;
		//((alarm_service_user_code_t *)(user_code_global_ptr + user_code_index))->user_code = user_code;
		memset(((alarm_service_user_code_t *)(user_code_global_ptr + user_code_index))->user_code,0,MAX_CODE_LEN_STR); /*!< User description default USER INDEX */
		//sprintf(((alarm_service_user_code_t *)(user_code_global_ptr + user_code_index))->user_code,"%s",user_code);
		//strcpy(((alarm_service_user_code_t *)(user_code_global_ptr + user_code_index))->user_code,user_code);
		memcpy(((alarm_service_user_code_t *)(user_code_global_ptr + user_code_index))->user_code,user_code,user_code_len);
		/*Open RF Remote file:*/
		fp_user_code = fopen(CRED_USER_FILE_PATH, "r+");
		
		fseek(fp_user_code, (sizeof(alarm_service_user_code_t)*user_code_index), SEEK_SET);
		
		writebyte = fwrite(((alarm_service_user_code_t *)(user_code_global_ptr + user_code_index)), (sizeof(alarm_service_user_code_t)), 1, fp_user_code);
		
		ALARM_TRACE_INFO("user_index 					= %d\n", ((alarm_service_user_code_t *)(user_code_global_ptr + user_code_index))->user_index);
		ALARM_TRACE_INFO("user_status 					= %d\n", ((alarm_service_user_code_t *)(user_code_global_ptr + user_code_index))->user_status);
		//ALARM_TRACE_INFO("user_code 					= %d\n", ((alarm_service_user_code_t *)(user_code_global_ptr + user_code_index))->user_code);
		ALARM_TRACE_INFO("user_code 					= %s\n", ((alarm_service_user_code_t *)(user_code_global_ptr + user_code_index))->user_code);
		ALARM_TRACE_INFO("user_profile_index 			= %d\n", ((alarm_service_user_code_t *)(user_code_global_ptr + user_code_index))->user_profile_index);
		ALARM_TRACE_INFO("user_schedule 				= %d\n", ((alarm_service_user_code_t *)(user_code_global_ptr + user_code_index))->user_schedule);
		ALARM_TRACE_INFO("user_area						= 0x%x\n", ((alarm_service_user_code_t *)(user_code_global_ptr + user_code_index))->user_area);
		ALARM_TRACE_INFO("user_control_devices			= 0x%x\n", ((alarm_service_user_code_t *)(user_code_global_ptr + user_code_index))->user_control_devices);
		ALARM_TRACE_INFO("user_description				= %s\n", ((alarm_service_user_code_t *)(user_code_global_ptr + user_code_index))->user_description);
		
		if(fp_user_code)
		fclose(fp_user_code);
			/* ADD Notify user code updated */
	}
	return err;
}

/**
 * \fn CRED_MW_Errors_t load_installer_code_information(void)
 * \brief load installer global structure from NAND flash.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t load_installer_code_information(void)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	int i;
	FILE *fp_installer_code=NULL; 
	/*Open installer Code file:*/
	fp_installer_code = fopen(CRED_INSTALLER_FILE_PATH, "r");
	if(fp_installer_code == NULL)
	{
		/*File Does Not Exit => Create the File*/
		fp_installer_code = fopen(CRED_INSTALLER_FILE_PATH, "w+");
		
		for(i=0;i<=MAX_INSTALLER_CODE;i++)
		{
			((alarm_service_installer_code_t *)(installer_code_global_ptr + i))->installer_index = i;
			if(i == 1){
				
				((alarm_service_installer_code_t *)(installer_code_global_ptr + i))->installer_status = 1; /*!<  Only installer 001 enabled by default*/
				((alarm_service_installer_code_t *)(installer_code_global_ptr + i))->installer_permission = 1; /*!<  Only installer 001 enabled by default*/
				//((alarm_service_installer_code_t *)(installer_code_global_ptr + i))->installer_code= 1234; /*!<  installer 1 default code 1234*/
				memset(((alarm_service_installer_code_t *)(installer_code_global_ptr + i))->installer_code,0,MAX_CODE_LEN_STR); /*!< installer description default installer INDEX */
				sprintf(((alarm_service_installer_code_t *)(installer_code_global_ptr + i))->installer_code,"1234",0);
				
			}else
			{
				((alarm_service_installer_code_t *)(installer_code_global_ptr + i))->installer_status = 0;
				((alarm_service_installer_code_t *)(installer_code_global_ptr + i))->installer_permission = 0;
				//((alarm_service_installer_code_t *)(installer_code_global_ptr + i))->installer_code= 0000; /*!<  installer 2 default code 1234*/
				memset(((alarm_service_installer_code_t *)(installer_code_global_ptr + i))->installer_code,0,MAX_CODE_LEN_STR); /*!< installer description default installer INDEX */
				sprintf(((alarm_service_installer_code_t *)(installer_code_global_ptr + i))->installer_code,"1234",0);
			}
			
			
			memset(((alarm_service_installer_code_t *)(installer_code_global_ptr + i))->installer_description,0,CRED_USER_DESCRIPTION_STR); /*!< installer description default installer INDEX */
			sprintf(((alarm_service_installer_code_t *)(installer_code_global_ptr + i))->installer_description,"INSTALLER %d",i);

		}
		
		fwrite(installer_code_global_ptr, (sizeof(alarm_service_installer_code_t)*(MAX_INSTALLER_CODE+1)), 1, fp_installer_code);
		ALARM_TRACE_INFO("Create installer code File for saving installer code information \n ");
	}
	else
	{
		/*File Exist Read Old Information*/
		fread(installer_code_global_ptr, (sizeof(alarm_service_installer_code_t)*(MAX_INSTALLER_CODE+1)), 1, fp_installer_code);
		ALARM_TRACE_INFO("Read installer Code Saved  Information \n ");
	}
	/*Close installer Code file*/
	fclose(fp_installer_code);
	return err;
}
/**
 * \fn void alarm_factory_reset_installer(void)
 * \brief Factory reset installer.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_factory_reset_installer(void)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	int i;
	FILE *fp_installer_code=NULL; 
	/*Open installer Code file:*/
	ALARM_TRACE_INFO("START: alarm_factory_reset_installer \n");
	fp_installer_code = fopen(CRED_INSTALLER_FILE_PATH, "r+");
	if(fp_installer_code == NULL)
	{
		/*File Does Not Exit => Create the File*/
		fp_installer_code = fopen(CRED_INSTALLER_FILE_PATH, "w+");
	
	}
	for(i=1;i<=MAX_INSTALLER_CODE;i++)
	{
			((alarm_service_installer_code_t *)(installer_code_global_ptr + i))->installer_index = i;
			if(i == 1){
				
				((alarm_service_installer_code_t *)(installer_code_global_ptr + i))->installer_status = 1; /*!<  Only installer 001 enabled by default*/
				((alarm_service_installer_code_t *)(installer_code_global_ptr + i))->installer_permission = 1; /*!<  Only installer 001 enabled by default*/
				//((alarm_service_installer_code_t *)(installer_code_global_ptr + i))->installer_code= 1234; /*!<  installer 1 default code 1234*/
				memset(((alarm_service_installer_code_t *)(installer_code_global_ptr + i))->installer_code,0,MAX_CODE_LEN_STR); /*!< installer description default installer INDEX */
				sprintf(((alarm_service_installer_code_t *)(installer_code_global_ptr + i))->installer_code,"1234",0);
				
			}else
			{
				((alarm_service_installer_code_t *)(installer_code_global_ptr + i))->installer_status = 0;
				((alarm_service_installer_code_t *)(installer_code_global_ptr + i))->installer_permission = 0;
				//((alarm_service_installer_code_t *)(installer_code_global_ptr + i))->installer_code= 0000; /*!<  installer 2 default code 1234*/
				memset(((alarm_service_installer_code_t *)(installer_code_global_ptr + i))->installer_code,0,MAX_CODE_LEN_STR); /*!< installer description default installer INDEX */
				sprintf(((alarm_service_installer_code_t *)(installer_code_global_ptr + i))->installer_code,"1234",0);
			}
			
			
			memset(((alarm_service_installer_code_t *)(installer_code_global_ptr + i))->installer_description,0,CRED_USER_DESCRIPTION_STR); /*!< installer description default installer INDEX */
			sprintf(((alarm_service_installer_code_t *)(installer_code_global_ptr + i))->installer_description,"INSTALLER %d",i);

	}
	
	fwrite(installer_code_global_ptr, (sizeof(alarm_service_installer_code_t)*(MAX_INSTALLER_CODE+1)), 1, fp_installer_code);
	
	/*Close installer Code file*/
	fclose(fp_installer_code);
	ALARM_TRACE_INFO("END: alarm_factory_reset_installer \n");
	return err;
}
/**
 * \fn CRED_MW_Errors_t check_installer_code(uint32_t installer_code,uint32_t *installer_code_index, uint32_t *installer_code_ptr)
 * \brief get RF zone  informations.
 *
 * \param zone_index: zone index.
 * \return No parameter.
 */

CRED_MW_Errors_t check_installer_code(char *installer_code,uint32_t *installer_code_index, uint32_t *installer_code_ptr)
{
	
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	uint8_t installer_flag = 0;
	int i;
	for(i=1;i<=MAX_INSTALLER_CODE;i++)
	{
			//if(((alarm_service_installer_code_t *)(installer_code_global_ptr + i))->installer_code == installer_code)
			if(strcmp(((alarm_service_installer_code_t *)(installer_code_global_ptr + i))->installer_code,installer_code) == 0)
			{
				
				if(((alarm_service_installer_code_t *)(installer_code_global_ptr + i))->installer_status == 1)
				{
					*installer_code_index = i;
					*installer_code_ptr = ((alarm_service_installer_code_t *)(installer_code_global_ptr + i));
					ALARM_TRACE_INFO("installer CODE %s entred : OK index %d \n", installer_code,i);
					installer_flag = 1;
					ALARM_TRACE_INFO("installer_index 					= %d\n", ((alarm_service_installer_code_t *)(installer_code_global_ptr + i))->installer_index);
					ALARM_TRACE_INFO("installer_status 					= %d\n", ((alarm_service_installer_code_t *)(installer_code_global_ptr + i))->installer_status);
					ALARM_TRACE_INFO("installer_permission 				= %d\n", ((alarm_service_installer_code_t *)(installer_code_global_ptr + i))->installer_permission);
					//ALARM_TRACE_INFO("installer_code 					= %d\n", ((alarm_service_installer_code_t *)(installer_code_global_ptr + i))->installer_code);
					ALARM_TRACE_INFO("installer_code 					= %s\n", ((alarm_service_installer_code_t *)(installer_code_global_ptr + i))->installer_code);
					ALARM_TRACE_INFO("installer_description				= %s\n", ((alarm_service_installer_code_t *)(installer_code_global_ptr + i))->installer_description);
					memcpy(&(current_installer_code_str.installer_index),((alarm_service_installer_code_t *)(installer_code_global_ptr + i)),(sizeof(alarm_service_installer_code_t)));
					
				}else
				{
					ALARM_TRACE_INFO("INSTALLER CODE %d entred : OK index %d !!!! NOT ACTIVATED \n", installer_code,i);
					installer_flag = 2;
				}
				
				i = MAX_INSTALLER_CODE + 1;
			}
			if(installer_flag !=0 )
			{
				break;
			}
	}
	if((installer_flag == 0) || (installer_flag == 2))
	{
		*installer_code_index = 0;
		*installer_code_ptr = NULL;
		ALARM_TRACE_INFO("WRONG !!!!! INSTALLER CODE %s entred \n", installer_code);
		/* ADD NOTIFY */
	}
	return err;
}

/**
 * \fn CRED_MW_Errors_t change_installer_code(uint32_t installer_code,uint8_t installer_code_index)
 * \brief get RF zone  informations.
 *
 * \param zone_index: zone index.
 * \return No parameter.
 */

CRED_MW_Errors_t change_installer_code(char *installer_code,uint32_t *installer_code_app_index)
{
	
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	FILE *fp_installer_code=NULL;
	int writebyte = 0;
	int i,installer_code_index;
	int installer_code_len =0;
	uint8_t installer_flag = 0;
	
	installer_code_len  = strlen(installer_code);
	
	ALARM_TRACE_INFO("INSTALLER CODE Len : %d \n", installer_code_len);
	
	if(installer_code_len > MAX_CODE_LEN)
	{
		installer_code_len = MAX_CODE_LEN;
	}
	//ALARM_TRACE_INFO("1-INSTALLER CODE Len : %d \n", installer_code_len);
	installer_code_index = *installer_code_app_index;
	//ALARM_TRACE_INFO("2-INSTALLER CODE Len : %d \n", installer_code_len);
	
	if((installer_code_index > MAX_INSTALLER_CODE) || (installer_code_index < 1 ) || (installer_code_len < MIN_CODE_LEN))
	{
		ALARM_TRACE_ERROR("unsupported installer code %s or index: %d \n",installer_code, installer_code_index);
		*installer_code_app_index = 0;
		installer_flag = 1;
		return CRED_MW_NO_ERROR;
	}
	//ALARM_TRACE_INFO("3-INSTALLER CODE Len : %d \n", installer_code_len);
	for(i=1;i<=MAX_INSTALLER_CODE;i++)
	{
			//if((((alarm_service_installer_code_t *)(installer_code_global_ptr + i))->installer_code == installer_code) && (((alarm_service_installer_code_t *)(installer_code_global_ptr + i))->installer_status == 1))
			if((strcmp(((alarm_service_installer_code_t *)(installer_code_global_ptr + i))->installer_code,installer_code) == 0) && (((alarm_service_installer_code_t *)(installer_code_global_ptr + i))->installer_status == 1))	
			{
				
				ALARM_TRACE_INFO("MAX_INSTALLER CODE %s entred : Used\n", installer_code);
				i = MAX_INSTALLER_CODE + 1;
				*installer_code_app_index = 0;
				installer_flag = 1;
			}
			if(installer_flag == 1)
			{
				break;
			}
	}
	if(installer_flag == 0){
		 installer_code_index = *installer_code_app_index;
		 
		
		//((alarm_service_installer_code_t *)(installer_code_global_ptr + installer_code_index))->installer_code = installer_code;
		memset(((alarm_service_installer_code_t *)(installer_code_global_ptr + installer_code_index))->installer_code,0,MAX_CODE_LEN_STR); /*!< User description default USER INDEX */
		//sprintf(((alarm_service_installer_code_t *)(installer_code_global_ptr + installer_code_index))->installer_code,"%s",installer_code);
		//strcpy(((alarm_service_installer_code_t *)(installer_code_global_ptr + installer_code_index))->installer_code,installer_code);
		memcpy(((alarm_service_installer_code_t *)(installer_code_global_ptr + installer_code_index))->installer_code,installer_code,installer_code_len);
		
		/*Open RF Remote file:*/
		fp_installer_code = fopen(CRED_INSTALLER_FILE_PATH, "r+");
		
		fseek(fp_installer_code, (sizeof(alarm_service_installer_code_t)*installer_code_index), SEEK_SET);
		
		writebyte = fwrite(((alarm_service_installer_code_t *)(installer_code_global_ptr + installer_code_index)), (sizeof(alarm_service_installer_code_t)), 1, fp_installer_code);
		
		ALARM_TRACE_INFO("installer_index 					= %d\n", ((alarm_service_installer_code_t *)(installer_code_global_ptr + installer_code_index))->installer_index);
		ALARM_TRACE_INFO("installer_status 					= %d\n", ((alarm_service_installer_code_t *)(installer_code_global_ptr + installer_code_index))->installer_status);
		ALARM_TRACE_INFO("installer_permission				= %d\n", ((alarm_service_installer_code_t *)(installer_code_global_ptr + installer_code_index))->installer_permission);
		//ALARM_TRACE_INFO("installer_code 					= %d\n", ((alarm_service_installer_code_t *)(installer_code_global_ptr + installer_code_index))->installer_code);
		ALARM_TRACE_INFO("installer_code 					= %s\n", ((alarm_service_installer_code_t *)(installer_code_global_ptr + installer_code_index))->installer_code);
		ALARM_TRACE_INFO("installer_description				= %s\n", ((alarm_service_installer_code_t *)(installer_code_global_ptr + installer_code_index))->installer_description);
		
		if(fp_installer_code)
		fclose(fp_installer_code);
			/* ADD Notify installer code updated */
	}
	//ALARM_TRACE_INFO("End-INSTALLER CODE Len : %d \n", installer_code_len);
	return err;
}

/**
 * \fn CRED_MW_Errors_t load_profile_information(void)
 * \brief load profile global structure from NAND flash.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t load_profile_information(void)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	int i;
	FILE *fp_profile=NULL;

	/*Open profile Code file:*/
	fp_profile = fopen(CRED_PROFILE_FILE_PATH, "r");
	if(fp_profile == NULL)
	{
		/*File Does Not Exit => Create the File*/
		fp_profile = fopen(CRED_PROFILE_FILE_PATH, "w+");
		
		for(i=0;i<=MAX_PROFILE_NUMBER;i++)
		{
			((alarm_service_profile_t *)(profile_global_ptr + i))->profile_index = i;	/*!<  profile : index (number 1 : 64)*/
			memset(((alarm_service_profile_t *)(profile_global_ptr + i))->profile_description,0,CRED_PROFILE_DESCRIPTION_STR);/*!< User description. */
			if(i == 1)
			{
				((alarm_service_profile_t *)(profile_global_ptr + i))->profile_status = CRED_ENABBLE;	/*!<  profile :  status*/
				sprintf(((alarm_service_profile_t *)(profile_global_ptr + i))->profile_description,"Main user",i);
				((alarm_service_profile_t *)(profile_global_ptr + i))->profile_access_1 = 0xFFFFFFFF;
				((alarm_service_profile_t *)(profile_global_ptr + i))->profile_access_2 = 0xFFFFFFFF;
				((alarm_service_profile_t *)(profile_global_ptr + i))->profile_live_cam_1 = 0xFFFFFFFF;
				((alarm_service_profile_t *)(profile_global_ptr + i))->profile_live_cam_2 = 0xFFFFFFFF;
				
			}else
			{
				sprintf(((alarm_service_profile_t *)(profile_global_ptr + i))->profile_description,"Profile %02d",i);
				((alarm_service_profile_t *)(profile_global_ptr + i))->profile_status = CRED_DISABLE;	/*!<  profile :  status*/
				((alarm_service_profile_t *)(profile_global_ptr + i))->profile_access_1 = 0;
				((alarm_service_profile_t *)(profile_global_ptr + i))->profile_access_2 = 0;
				((alarm_service_profile_t *)(profile_global_ptr + i))->profile_live_cam_1 = 0;
				((alarm_service_profile_t *)(profile_global_ptr + i))->profile_live_cam_2 = 0;
			}
		}
		
		fwrite(profile_global_ptr, (sizeof(alarm_service_profile_t)*(MAX_PROFILE_NUMBER+1)), 1, fp_profile);
		ALARM_TRACE_INFO("Create profile File for saving profile information \n ");
	}
	else
	{
		/*File Exist Read Old Information*/
		fread(profile_global_ptr, (sizeof(alarm_service_profile_t)*(MAX_PROFILE_NUMBER+1)), 1, fp_profile);
		ALARM_TRACE_INFO("Read profile Saved  Information \n ");
	}

	/*Close profile Code file*/
	fclose(fp_profile);
	return err;
}

/**
 * \fn CRED_MW_Errors_t alarm_save_profile_information(uint8_t profile_index,alarm_service_profile_t *profile_tmp_ptr)
 * \brief save one profile structure to global str and NAND flash.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_save_profile_information(uint32_t *profile_index_app,alarm_service_profile_t *profile_tmp_ptr)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	int i;
	FILE *fp_profile=NULL; 
	int writebyte = 0;
	int profile_index;
	alarm_service_profile_t *profile_ptr;
	profile_index = *profile_index_app;

	if(profile_index > MAX_PROFILE_NUMBER || profile_index < 1)
	{
		ALARM_TRACE_ERROR("unsupported profile index : %d \n",profile_index);
		*profile_index_app=0;
		return CRED_MW_NO_ERROR;
	}
	if(profile_tmp_ptr == NULL)
	{
		ALARM_TRACE_ERROR("unsupported profile pointer : %d \n",profile_tmp_ptr);
		*profile_index_app=0;
		return CRED_MW_NO_ERROR;
	}
	/*Open profile Code file:*/
	fp_profile = fopen(CRED_PROFILE_FILE_PATH, "r+");
	if(fp_profile == NULL)
	{
		ALARM_TRACE_ERROR("RECIVER FILE ERROR : %s \n" CRED_PROFILE_FILE_PATH);
	}
	else
	{
		/*File Exist */
		fseek(fp_profile, (sizeof(alarm_service_profile_t)*profile_index), SEEK_SET);
		memcpy(((alarm_service_profile_t *)(profile_global_ptr + profile_index)),profile_tmp_ptr,sizeof(alarm_service_profile_t));
		((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_index = profile_index;
		((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_status = CRED_ENABBLE;
		writebyte = fwrite(((alarm_service_profile_t *)(profile_global_ptr + profile_index)), (sizeof(alarm_service_profile_t)), 1, fp_profile);
		
		ALARM_TRACE_INFO("profile_index 					= %d\n", ((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_index);
		ALARM_TRACE_INFO("profile_index 					= %s\n", ((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_description);
		ALARM_TRACE_INFO("profile_status 					= %d\n", ((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_status);
		ALARM_TRACE_INFO("profile_access_1 					= 0x%x\n", ((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_access_1);
		ALARM_TRACE_INFO("profile_access_2 					= 0x%x\n", ((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_access_2);
		ALARM_TRACE_INFO("profile_live_cam_1				= 0x%x\n", ((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_live_cam_1);
		ALARM_TRACE_INFO("profile_live_cam_2				= 0x%x\n", ((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_live_cam_2);
	}
	/*Close Receiver Code file*/
	fclose(fp_profile);
	return err;
}
/**
 * \fn CRED_MW_Errors_t alarm_set_default_profile_information(uint8_t profile_index)
 * \brief set default and save one profile structure to global str and NAND flash.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_set_default_profile_information(uint32_t *profile_index_app)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	int i;
	FILE *fp_profile=NULL; 
	int writebyte = 0;
	int profile_index;
	profile_index = *profile_index_app;

	if(profile_index > MAX_PROFILE_NUMBER || profile_index < 1)
	{
		ALARM_TRACE_ERROR("unsupported profile index : %d \n",profile_index);
		*profile_index_app=0;
		return CRED_MW_NO_ERROR;
	}
	
	/*Open profile Code file:*/
	fp_profile = fopen(CRED_PROFILE_FILE_PATH, "r+");
	if(fp_profile == NULL)
	{
		ALARM_TRACE_ERROR("RECIVER FILE ERROR : %s \n" CRED_PROFILE_FILE_PATH);
	}
	else
	{
		/*File Exist */
		fseek(fp_profile, (sizeof(alarm_service_profile_t)*profile_index), SEEK_SET);
		if(profile_index == 1)
		{
				((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_status = CRED_ENABBLE;	/*!<  profile :  status*/
				sprintf(((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_description,"Main user",i);
				((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_access_1 = 0xFFFFFFFF;
				((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_access_2 = 0xFFFFFFFF;
				((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_live_cam_1 = 0xFFFFFFFF;
				((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_live_cam_2 = 0xFFFFFFFF;
				((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_index = profile_index;
				((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_status = CRED_ENABBLE;
				
		}else
		{
				sprintf(((alarm_service_profile_t *)(profile_global_ptr + i))->profile_description,"Profile %02d",i);
				((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_status = CRED_DISABLE;	/*!<  profile :  status*/
				((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_access_1 = 0;
				((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_access_2 = 0;
				((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_live_cam_1 = 0;
				((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_live_cam_2 = 0;
				((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_index = profile_index;
				((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_status = CRED_DISABLE;
		}

		writebyte = fwrite(((alarm_service_profile_t *)(profile_global_ptr + profile_index)), (sizeof(alarm_service_profile_t)), 1, fp_profile);
		
		ALARM_TRACE_INFO("profile_index 					= %d\n", ((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_index);
		ALARM_TRACE_INFO("profile_index 					= %s\n", ((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_description);
		ALARM_TRACE_INFO("profile_status 					= %d\n", ((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_status);
		ALARM_TRACE_INFO("profile_access_1 					= 0x%x\n", ((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_access_1);
		ALARM_TRACE_INFO("profile_access_2 					= 0x%x\n", ((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_access_2);
		ALARM_TRACE_INFO("profile_live_cam_1				= 0x%x\n", ((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_live_cam_1);
		ALARM_TRACE_INFO("profile_live_cam_2				= 0x%x\n", ((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_live_cam_2);
	}
	/*Close Receiver Code file*/
	fclose(fp_profile);
	return err;
}
/**
 * \fn CRED_MW_Errors_t alarm_clear_profile_information(uint8_t profile_index)
 * \brief clear and save one profile structure to global str and NAND flash.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_clear_profile_information(uint32_t *profile_index_app)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	int i;
	FILE *fp_profile=NULL; 
	int writebyte = 0;
	int profile_index;
	profile_index = *profile_index_app;

	if(profile_index > MAX_PROFILE_NUMBER || profile_index < 1)
	{
		ALARM_TRACE_ERROR("unsupported profile index : %d \n",profile_index);
		*profile_index_app=0;
		return CRED_MW_NO_ERROR;
	}
	
	/*Open profile Code file:*/
	fp_profile = fopen(CRED_PROFILE_FILE_PATH, "r+");
	if(fp_profile == NULL)
	{
		ALARM_TRACE_ERROR("RECIVER FILE ERROR : %s \n" CRED_PROFILE_FILE_PATH);
	}
	else
	{
		/*File Exist */
		
		sprintf(((alarm_service_profile_t *)(profile_global_ptr + i))->profile_description,"Profile %02d",i);
		((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_status = CRED_DISABLE;	/*!<  profile :  status*/
		((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_access_1 = 0;
		((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_access_2 = 0;
		((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_live_cam_1 = 0;
		((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_live_cam_2 = 0;
		((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_index = profile_index;
		((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_status = CRED_DISABLE;
	
		writebyte = fwrite(((alarm_service_profile_t *)(profile_global_ptr + profile_index)), (sizeof(alarm_service_profile_t)), 1, fp_profile);
		
		ALARM_TRACE_INFO("profile_index 					= %d\n", ((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_index);
		ALARM_TRACE_INFO("profile_index 					= %s\n", ((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_description);
		ALARM_TRACE_INFO("profile_status 					= %d\n", ((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_status);
		ALARM_TRACE_INFO("profile_access_1 					= 0x%x\n", ((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_access_1);
		ALARM_TRACE_INFO("profile_access_2 					= 0x%x\n", ((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_access_2);
		ALARM_TRACE_INFO("profile_live_cam_1				= 0x%x\n", ((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_live_cam_1);
		ALARM_TRACE_INFO("profile_live_cam_2				= 0x%x\n", ((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_live_cam_2);
	}
	/*Close Receiver Code file*/
	fclose(fp_profile);
	return err;
}
/**
 * \fn CRED_MW_Errors_t alarm_get_profile_information(uint8_t profile_index,alarm_service_profile_t *profile_tmp_ptr)
 * \brief save one profile structure to global str and NAND flash.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_get_profile_information(uint32_t *profile_index_app,alarm_service_profile_t *profile_tmp_ptr)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	int profile_index;
	alarm_service_profile_t *profile_ptr;
	profile_index = *profile_index_app;
	
	if(profile_index > MAX_PROFILE_NUMBER || profile_index < 1)
	{
		ALARM_TRACE_ERROR("unsupported profile index : %d \n",profile_index);
		*profile_index_app=0;
		return CRED_MW_NO_ERROR;
	}
	if(profile_tmp_ptr == NULL)
	{
		ALARM_TRACE_ERROR("unsupported profile pointer : %d \n",profile_tmp_ptr);
		*profile_index_app=0;
		return CRED_MW_NO_ERROR;
	}

	memcpy(profile_tmp_ptr,((alarm_service_profile_t *)(profile_global_ptr + profile_index)),sizeof(alarm_service_profile_t));

	ALARM_TRACE_INFO("profile_index 					= %d\n", ((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_index);
	ALARM_TRACE_INFO("profile_index 					= %s\n", ((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_description);
	ALARM_TRACE_INFO("profile_status 					= %d\n", ((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_status);
	ALARM_TRACE_INFO("profile_access_1 					= 0x%x\n", ((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_access_1);
	ALARM_TRACE_INFO("profile_access_2 					= 0x%x\n", ((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_access_2);
	ALARM_TRACE_INFO("profile_live_cam_1				= 0x%x\n", ((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_live_cam_1);
	ALARM_TRACE_INFO("profile_live_cam_2				= 0x%x\n", ((alarm_service_profile_t *)(profile_global_ptr + profile_index))->profile_live_cam_2);

	return err;
}
/**
 * \fn CRED_MW_Errors_t load_profile_information(void)
 * \brief load profile global structure from NAND flash.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_factory_reset_profile(void)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	int i;
	FILE *fp_profile=NULL;

	/*Open profile Code file:*/
	fp_profile = fopen(CRED_PROFILE_FILE_PATH, "r+");
	if(fp_profile == NULL)
	{
		/*File Does Not Exit => Create the File*/
		fp_profile = fopen(CRED_PROFILE_FILE_PATH, "w+");
	}

	for(i=0;i<=MAX_PROFILE_NUMBER;i++)
	{
		((alarm_service_profile_t *)(profile_global_ptr + i))->profile_index = i;	/*!<  profile : index (number 1 : 64)*/
		memset(((alarm_service_profile_t *)(profile_global_ptr + i))->profile_description,0,CRED_PROFILE_DESCRIPTION_STR);/*!< User description. */
		if(i == 1)
		{
			((alarm_service_profile_t *)(profile_global_ptr + i))->profile_status = CRED_ENABBLE;	/*!<  profile :  status*/
			sprintf(((alarm_service_profile_t *)(profile_global_ptr + i))->profile_description,"Main user",i);
			((alarm_service_profile_t *)(profile_global_ptr + i))->profile_access_1 = 0xFFFFFFFF;
			((alarm_service_profile_t *)(profile_global_ptr + i))->profile_access_2 = 0xFFFFFFFF;
			((alarm_service_profile_t *)(profile_global_ptr + i))->profile_live_cam_1 = 0xFFFFFFFF;
			((alarm_service_profile_t *)(profile_global_ptr + i))->profile_live_cam_2 = 0xFFFFFFFF;
				
		}else
		{
			sprintf(((alarm_service_profile_t *)(profile_global_ptr + i))->profile_description,"Profile %02d",i);
			((alarm_service_profile_t *)(profile_global_ptr + i))->profile_status = CRED_DISABLE;	/*!<  profile :  status*/
			((alarm_service_profile_t *)(profile_global_ptr + i))->profile_access_1 = 0;
			((alarm_service_profile_t *)(profile_global_ptr + i))->profile_access_2 = 0;
			((alarm_service_profile_t *)(profile_global_ptr + i))->profile_live_cam_1 = 0;
			((alarm_service_profile_t *)(profile_global_ptr + i))->profile_live_cam_2 = 0;
		}
	}

	fwrite(profile_global_ptr, (sizeof(alarm_service_profile_t)*(MAX_PROFILE_NUMBER+1)), 1, fp_profile);
	ALARM_TRACE_INFO("Create profile File for saving profile information \n ");

	/*Close profile Code file*/
	fclose(fp_profile);
	return err;
}
/**
 * \fn CRED_MW_Errors_t load_receiver_information(void)
 * \brief load receiver global structure from NAND flash.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t load_receiver_information(void)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	int i;
	FILE *fp_receiver=NULL;
	alarm_service_receiver_t *receiver_ptr;
	/*Open Receiver Code file:*/
	fp_receiver = fopen(CRED_RECEIVER_FILE_PATH, "r");
	if(fp_receiver == NULL)
	{
		/*File Does Not Exit => Create the File*/
		fp_receiver = fopen(CRED_RECEIVER_FILE_PATH, "w+");
		
		for(i=0;i<=MAX_RECEIVER_NUMBER;i++)
		{
			((alarm_service_receiver_t *)(receiver_global_ptr + i))->receiver_index = i;	/*!<  receiver : index (number 1 : 64)*/
			((alarm_service_receiver_t *)(receiver_global_ptr + i))->receiver_status = CRED_DISABLE;	/*!<  receiver : satatus (1: enabled 0 : disabled)*/
			((alarm_service_receiver_t *)(receiver_global_ptr + i))->receiver_type = CRED_RECEIVER_TYPE_NOT_USED;      /*!<  receiver type :   ARC or Contact)*/
			((alarm_service_receiver_t *)(receiver_global_ptr + i))->receiver_protocols = CRED_PROTOCOL_NOT_USED; /*!<  receiver protocols : Voice (GSM), SMS,MMS,E-mail,APP Push Notification : (A8A7A6A5A4A3A2A1: 0/1) */
			((alarm_service_receiver_t *)(receiver_global_ptr + i))->receiver_arc_protocols = CRED_ARC_PROTOCOL_NOT_USED; /*!<  receiver arc protocols : SIA IP,2 SIA Av 01,SIA SMS,Contact ID IP,Contact ID Voice channel,Contact ID SMS : (A8A7A6A5A4A3A2A1: 0/1)*/
			((alarm_service_receiver_t *)(receiver_global_ptr + i))->receiver_email_primary_channel = CRED_TCIP_IP_CHANNEL;	/*!<  Receiver : Email primary channel : 3G  or TCP-IP */
			((alarm_service_receiver_t *)(receiver_global_ptr + i))->receiver_app_primary_channel = CRED_TCIP_IP_CHANNEL;	/*!<  Receiver : App primary channel : 3G  or TCP-IP*/
			((alarm_service_receiver_t *)(receiver_global_ptr + i))->receiver_arc_primary_channel = CRED_TCIP_IP_CHANNEL;	/*!<  Receiver : Email primary channel : 3G  or TCP-IP*/
			((alarm_service_receiver_t *)(receiver_global_ptr + i))->receiver_arc_account_number=0;	/*!<  Receiver : arc accout number*/
			((alarm_service_receiver_t *)(receiver_global_ptr + i))->receiver_arc_ip_id=123456;	/*!<  Receiver : arc id number*/
			((alarm_service_receiver_t *)(receiver_global_ptr + i))->receiver_backup_enable = CRED_DISABLE;	/*!<  Receiver : enabling a backup channel for Email, App and ARC 0 or 1*/
			((alarm_service_receiver_t *)(receiver_global_ptr + i))->receiver_2way_audio_enable = CRED_DISABLE;	/*!<  Receiver : Two Way Audio Enable 0 or 1*/
			memset(((alarm_service_receiver_t *)(receiver_global_ptr + i))->receiver_description,0,CRED_RECEIVER_DESCRIPTION_STR);/*!< User description. */
			sprintf(((alarm_service_receiver_t *)(receiver_global_ptr + i))->receiver_description,"RECEIVER %02d",i);
			memset(((alarm_service_receiver_t *)(receiver_global_ptr + i))->receiver_phone_number,0,MAX_PHONE_NUMBER_STR);/*!< Receiver phone number */
			memset(((alarm_service_receiver_t *)(receiver_global_ptr + i))->receiver_sms_number,0,MAX_PHONE_NUMBER_STR);/*!< Receiver SMS number */
			memset(((alarm_service_receiver_t *)(receiver_global_ptr + i))->receiver_email_address,0,MAX_EMAIL_ADDRESS_STR);/*!< Receiver Email address */
			memset(((alarm_service_receiver_t *)(receiver_global_ptr + i))->receiver_ip_address,0,MAX_IP_ADDRESS_STR);/*!< Receiver IP address */
			((alarm_service_receiver_t *)(receiver_global_ptr + i))->receiver_ip_port = 0; /*!< Receiver IP Port */
		}
		
		fwrite(receiver_global_ptr, (sizeof(alarm_service_receiver_t)*(MAX_RECEIVER_NUMBER+1)), 1, fp_receiver);
		ALARM_TRACE_INFO("Create Receiver File for saving Receiver information \n ");
	}
	else
	{
		/*File Exist Read Old Information*/
		fread(receiver_global_ptr, (sizeof(alarm_service_receiver_t)*(MAX_RECEIVER_NUMBER+1)), 1, fp_receiver);
		ALARM_TRACE_INFO("Read Receiver Saved  Information \n ");
		
#ifndef CRED_ARC_DEFAULT
		for(i=0;i<=MAX_RECEIVER_NUMBER;i++)
		{
		
			/* tmp code for demo to read ARC info */
			receiver_ptr = ((alarm_service_receiver_t *)(receiver_global_ptr + i));
			
			if(receiver_ptr->receiver_type == CRED_RECEIVER_TYPE_ARC)
			{
				if(receiver_ptr->receiver_arc_protocols & CRED_ARC_PROTOCOL_CID_VOICE)
				{
					ALARM_TRACE_INFO("ARC  CRED_ARC_PROTOCOL_CID_VOICE \n ");
					memset(arc_number_cid,0,20);
					memcpy(arc_number_cid,receiver_ptr->receiver_phone_number,strlen(receiver_ptr->receiver_phone_number));
					arc_cid_voice_account = receiver_ptr->receiver_arc_account_number;
					ALARM_TRACE_INFO("ARC  CID Phone number :%s account %04d \n ",arc_number_cid,arc_cid_voice_account);
					arc_cid_voice_global = 1;
					
				}else if((receiver_ptr->receiver_arc_protocols & CRED_ARC_PROTOCOL_SIA_IP) || (receiver_ptr->receiver_arc_protocols & CRED_ARC_PROTOCOL_CID_IP))
				{
					ALARM_TRACE_INFO("ARC  CRED_ARC_PROTOCOL_SIA_IP & CRED_ARC_PROTOCOL_CID_IP \n ");
					memset(arc_ip_address,0,20);
					memcpy(arc_ip_address,receiver_ptr->receiver_ip_address,strlen(receiver_ptr->receiver_ip_address));
					arc_ip_account = receiver_ptr->receiver_arc_account_number;
					arc_ip_port = receiver_ptr->receiver_ip_port;
					ALARM_TRACE_INFO("ARC SIA/CID IP IP address :%s IP port %d account %04d \n ",arc_ip_address,arc_ip_port,arc_ip_account);
					arc_ip_global = 1;
					
				}else if((receiver_ptr->receiver_arc_protocols & CRED_ARC_PROTOCOL_SIA_SMS) || (receiver_ptr->receiver_arc_protocols & CRED_ARC_PROTOCOL_CID_SMS))
				{
					ALARM_TRACE_INFO("ARC  CRED_ARC_PROTOCOL_SIA_SMS & CRED_ARC_PROTOCOL_CID_SMS \n ");
					
					memset(arc_number_sms,0,20);
					memcpy(arc_number_sms,receiver_ptr->receiver_sms_number,strlen(receiver_ptr->receiver_sms_number));
					arc_sms_account = receiver_ptr->receiver_arc_account_number;
					ALARM_TRACE_INFO("ARC  SMS Phone number :%s account %04d \n ",arc_number_sms,arc_sms_account);
					arc_sms_global = 1;
				}
			}
		}
#endif
		
		
	}
	/*Close Receiver Code file*/
	fclose(fp_receiver);
	return err;
}

/**
 * \fn void alarm_factory_reset_receiver(void)
 * \brief Factory reset receiver.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_factory_reset_receiver(void)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	int i;
	FILE *fp_receiver=NULL;
	alarm_service_receiver_t *receiver_ptr;
	ALARM_TRACE_INFO("START: alarm_factory_reset_receiver \n");
	/*Open Receiver Code file:*/
	fp_receiver = fopen(CRED_RECEIVER_FILE_PATH, "r+");
	if(fp_receiver == NULL)
	{
		/*File Does Not Exit => Create the File*/
		fp_receiver = fopen(CRED_RECEIVER_FILE_PATH, "w+");
	}

	for(i=1;i<=MAX_RECEIVER_NUMBER;i++)
	{
		((alarm_service_receiver_t *)(receiver_global_ptr + i))->receiver_index = i;	/*!<  receiver : index (number 1 : 64)*/
		((alarm_service_receiver_t *)(receiver_global_ptr + i))->receiver_status = CRED_DISABLE;	/*!<  receiver : satatus (1: enabled 0 : disabled)*/
		((alarm_service_receiver_t *)(receiver_global_ptr + i))->receiver_type = CRED_RECEIVER_TYPE_NOT_USED;      /*!<  receiver type :   ARC or Contact)*/
		((alarm_service_receiver_t *)(receiver_global_ptr + i))->receiver_protocols = CRED_PROTOCOL_NOT_USED; /*!<  receiver protocols : Voice (GSM), SMS,MMS,E-mail,APP Push Notification : (A8A7A6A5A4A3A2A1: 0/1) */
		((alarm_service_receiver_t *)(receiver_global_ptr + i))->receiver_arc_protocols = CRED_ARC_PROTOCOL_NOT_USED; /*!<  receiver arc protocols : SIA IP,2 SIA Av 01,SIA SMS,Contact ID IP,Contact ID Voice channel,Contact ID SMS : (A8A7A6A5A4A3A2A1: 0/1)*/
		((alarm_service_receiver_t *)(receiver_global_ptr + i))->receiver_email_primary_channel = CRED_TCIP_IP_CHANNEL;	/*!<  Receiver : Email primary channel : 3G  or TCP-IP */
		((alarm_service_receiver_t *)(receiver_global_ptr + i))->receiver_app_primary_channel = CRED_TCIP_IP_CHANNEL;	/*!<  Receiver : App primary channel : 3G  or TCP-IP*/
		((alarm_service_receiver_t *)(receiver_global_ptr + i))->receiver_arc_primary_channel = CRED_TCIP_IP_CHANNEL;	/*!<  Receiver : Email primary channel : 3G  or TCP-IP*/
		((alarm_service_receiver_t *)(receiver_global_ptr + i))->receiver_arc_account_number=0;	/*!<  Receiver : arc accout number*/
		((alarm_service_receiver_t *)(receiver_global_ptr + i))->receiver_arc_ip_id=123456;	/*!<  Receiver : arc id number*/
		((alarm_service_receiver_t *)(receiver_global_ptr + i))->receiver_backup_enable = CRED_DISABLE;	/*!<  Receiver : enabling a backup channel for Email, App and ARC 0 or 1*/
		((alarm_service_receiver_t *)(receiver_global_ptr + i))->receiver_2way_audio_enable = CRED_DISABLE;	/*!<  Receiver : Two Way Audio Enable 0 or 1*/
		memset(((alarm_service_receiver_t *)(receiver_global_ptr + i))->receiver_description,0,CRED_RECEIVER_DESCRIPTION_STR);/*!< User description. */
		sprintf(((alarm_service_receiver_t *)(receiver_global_ptr + i))->receiver_description,"RECEIVER %02d",i);
		memset(((alarm_service_receiver_t *)(receiver_global_ptr + i))->receiver_phone_number,0,MAX_PHONE_NUMBER_STR);/*!< Receiver phone number */
		memset(((alarm_service_receiver_t *)(receiver_global_ptr + i))->receiver_sms_number,0,MAX_PHONE_NUMBER_STR);/*!< Receiver SMS number */
		memset(((alarm_service_receiver_t *)(receiver_global_ptr + i))->receiver_email_address,0,MAX_EMAIL_ADDRESS_STR);/*!< Receiver Email address */
		memset(((alarm_service_receiver_t *)(receiver_global_ptr + i))->receiver_ip_address,0,MAX_IP_ADDRESS_STR);/*!< Receiver IP address */
		((alarm_service_receiver_t *)(receiver_global_ptr + i))->receiver_ip_port = 0; /*!< Receiver IP Port */
	}
		
	fwrite(receiver_global_ptr, (sizeof(alarm_service_receiver_t)*(MAX_RECEIVER_NUMBER+1)), 1, fp_receiver);
	
	/*Close Receiver Code file*/
	fclose(fp_receiver);
	ALARM_TRACE_INFO("END: alarm_factory_reset_receiver \n");
	return err;
}

/**
 * \fn CRED_MW_Errors_t save_receiver_information(uint8_t receiver_index,alarm_service_receiver_t *receiver_tmp_ptr)
 * \brief save one receiver structure to global str and NAND flash.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t save_receiver_information(uint32_t *receiver_index_app,alarm_service_receiver_t *receiver_tmp_ptr)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	int i;
	FILE *fp_receiver=NULL; 
	int writebyte = 0;
	int receiver_index;
	alarm_service_receiver_t *receiver_ptr;
	receiver_index = *receiver_index_app;
	
	if(receiver_index > MAX_RECEIVER_NUMBER || receiver_index < 1)
	{
		ALARM_TRACE_ERROR("unsupported receiver index : %d \n",receiver_index);
		*receiver_index_app=0;
		return CRED_MW_NO_ERROR;
	}
	if(((alarm_service_receiver_t *)(receiver_global_ptr + receiver_index))->receiver_status == 1)
	{
		ALARM_TRACE_ERROR("Receiver index used: %d \n",receiver_index);
		*receiver_index_app=0;
		return CRED_MW_NO_ERROR;
	}
	/*Open Receiver Code file:*/
	fp_receiver = fopen(CRED_RECEIVER_FILE_PATH, "r+");
	if(fp_receiver == NULL)
	{
		ALARM_TRACE_ERROR("RECIVER FILE ERROR : %s \n" CRED_RECEIVER_FILE_PATH);
	}
	else
	{
		/*File Exist */
		fseek(fp_receiver, (sizeof(alarm_service_receiver_t)*receiver_index), SEEK_SET);
		memcpy(((alarm_service_receiver_t *)(receiver_global_ptr + receiver_index)),receiver_tmp_ptr,sizeof(alarm_service_receiver_t));
		((alarm_service_receiver_t *)(receiver_global_ptr + receiver_index))->receiver_index = receiver_index;
		((alarm_service_receiver_t *)(receiver_global_ptr + receiver_index))->receiver_status = CRED_ENABBLE;
		writebyte = fwrite(((alarm_service_receiver_t *)(receiver_global_ptr + receiver_index)), (sizeof(alarm_service_receiver_t)), 1, fp_receiver);
		
		ALARM_TRACE_INFO("receiver_index 					= %d\n", ((alarm_service_receiver_t *)(receiver_global_ptr + receiver_index))->receiver_index);
		ALARM_TRACE_INFO("receiver_status 					= %d\n", ((alarm_service_receiver_t *)(receiver_global_ptr + receiver_index))->receiver_status);
		ALARM_TRACE_INFO("receiver_type 					= %d\n", ((alarm_service_receiver_t *)(receiver_global_ptr + receiver_index))->receiver_type);
		ALARM_TRACE_INFO("receiver_protocols 				= 0x%x\n", ((alarm_service_receiver_t *)(receiver_global_ptr + receiver_index))->receiver_protocols);
		ALARM_TRACE_INFO("receiver_arc_protocols 			= 0x%x\n", ((alarm_service_receiver_t *)(receiver_global_ptr + receiver_index))->receiver_arc_protocols);
		ALARM_TRACE_INFO("receiver_email_primary_channel	= %d\n", ((alarm_service_receiver_t *)(receiver_global_ptr + receiver_index))->receiver_email_primary_channel);
		ALARM_TRACE_INFO("receiver_app_primary_channel	    = %d\n", ((alarm_service_receiver_t *)(receiver_global_ptr + receiver_index))->receiver_app_primary_channel);
		ALARM_TRACE_INFO("receiver_arc_primary_channel	    = %d\n", ((alarm_service_receiver_t *)(receiver_global_ptr + receiver_index))->receiver_arc_primary_channel);
		ALARM_TRACE_INFO("receiver_arc_account_number	    = %d\n", ((alarm_service_receiver_t *)(receiver_global_ptr + receiver_index))->receiver_arc_account_number);
		ALARM_TRACE_INFO("receiver_arc_ip_id	   			= %d\n", ((alarm_service_receiver_t *)(receiver_global_ptr + receiver_index))->receiver_arc_ip_id);
		ALARM_TRACE_INFO("receiver_backup_enable	   		= %d\n", ((alarm_service_receiver_t *)(receiver_global_ptr + receiver_index))->receiver_backup_enable);
		ALARM_TRACE_INFO("receiver_2way_audio_enable	   	= %d\n", ((alarm_service_receiver_t *)(receiver_global_ptr + receiver_index))->receiver_2way_audio_enable);
		ALARM_TRACE_INFO("receiver_description	   	        = %s\n", ((alarm_service_receiver_t *)(receiver_global_ptr + receiver_index))->receiver_description);
		ALARM_TRACE_INFO("receiver_phone_number	   	        = %s\n", ((alarm_service_receiver_t *)(receiver_global_ptr + receiver_index))->receiver_phone_number);
		ALARM_TRACE_INFO("receiver_sms_number	   	        = %s\n", ((alarm_service_receiver_t *)(receiver_global_ptr + receiver_index))->receiver_sms_number);
		ALARM_TRACE_INFO("receiver_email_address	   	    = %s\n", ((alarm_service_receiver_t *)(receiver_global_ptr + receiver_index))->receiver_email_address);
		ALARM_TRACE_INFO("receiver_ip_address	   	        = %s\n", ((alarm_service_receiver_t *)(receiver_global_ptr + receiver_index))->receiver_ip_address);
		ALARM_TRACE_INFO("receiver_ip_port	   	            = %d\n", ((alarm_service_receiver_t *)(receiver_global_ptr + receiver_index))->receiver_ip_port);
		ALARM_TRACE_INFO("Read Receiver %d Saved  Information \n ",receiver_index);
	}
	/* tmp code for demo to read ARC info */
	receiver_ptr = ((alarm_service_receiver_t *)(receiver_global_ptr + receiver_index));
	
	if(receiver_ptr->receiver_type == CRED_RECEIVER_TYPE_ARC)
	{
		if(receiver_ptr->receiver_arc_protocols & CRED_ARC_PROTOCOL_CID_VOICE)
		{
			ALARM_TRACE_INFO("ARC  CRED_ARC_PROTOCOL_CID_VOICE \n ");
			memset(arc_number_cid,0,20);
			memcpy(arc_number_cid,receiver_ptr->receiver_phone_number,strlen(receiver_ptr->receiver_phone_number));
			arc_cid_voice_account = receiver_ptr->receiver_arc_account_number;
			ALARM_TRACE_INFO("ARC  CID Phone number :%s account %04d \n ",arc_number_cid,arc_cid_voice_account);
			arc_cid_voice_global = 1;
			
		}else if((receiver_ptr->receiver_arc_protocols & CRED_ARC_PROTOCOL_SIA_IP) || (receiver_ptr->receiver_arc_protocols & CRED_ARC_PROTOCOL_CID_IP))
		{
			ALARM_TRACE_INFO("ARC  CRED_ARC_PROTOCOL_SIA_IP & CRED_ARC_PROTOCOL_CID_IP \n ");
			memset(arc_ip_address,0,20);
			memcpy(arc_ip_address,receiver_ptr->receiver_ip_address,strlen(receiver_ptr->receiver_ip_address));
			arc_ip_account = receiver_ptr->receiver_arc_account_number;
			arc_ip_port = receiver_ptr->receiver_ip_port;
			ALARM_TRACE_INFO("ARC SIA/CID IP IP address :%s IP port %d account %04d \n ",arc_ip_address,arc_ip_port,arc_ip_account);
			arc_ip_global = 1;
			
		}else if((receiver_ptr->receiver_arc_protocols & CRED_ARC_PROTOCOL_SIA_SMS) || (receiver_ptr->receiver_arc_protocols & CRED_ARC_PROTOCOL_CID_SMS))
		{
			ALARM_TRACE_INFO("ARC  CRED_ARC_PROTOCOL_SIA_SMS & CRED_ARC_PROTOCOL_CID_SMS \n ");
			
			memset(arc_number_sms,0,20);
			memcpy(arc_number_sms,receiver_ptr->receiver_sms_number,strlen(receiver_ptr->receiver_sms_number));
			arc_sms_account = receiver_ptr->receiver_arc_account_number;
			ALARM_TRACE_INFO("ARC  SMS Phone number :%s account %04d \n ",arc_number_sms,arc_sms_account);
			arc_sms_global = 1;
		}
	}
	
	/*Close Receiver Code file*/
	fclose(fp_receiver);
	return err;
}

/**
 * \fn CRED_MW_Errors_t delete_receiver_information(uint8_t receiver_index)
 * \brief save one receiver structure to global str and NAND flash.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t delete_receiver_information(uint32_t *receiver_index_app)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	int i;
	FILE *fp_receiver=NULL; 
	int writebyte = 0;
	int receiver_index;
	
	receiver_index = *receiver_index_app;
	
	if(receiver_index > MAX_RECEIVER_NUMBER || receiver_index < 1)
	{
		ALARM_TRACE_ERROR("unsupported receiver index : %d \n",receiver_index);
		receiver_index_app  = 0;
		return CRED_MW_NO_ERROR;
	}
	
	((alarm_service_receiver_t *)(receiver_global_ptr + receiver_index))->receiver_index = receiver_index;	/*!<  receiver : index (number 1 : 64)*/
	((alarm_service_receiver_t *)(receiver_global_ptr + receiver_index))->receiver_status = CRED_DISABLE;	/*!<  receiver : satatus (1: enabled 0 : disabled)*/
	((alarm_service_receiver_t *)(receiver_global_ptr + receiver_index))->receiver_type = CRED_RECEIVER_TYPE_NOT_USED;      /*!<  receiver type :   ARC or Contact)*/
	((alarm_service_receiver_t *)(receiver_global_ptr + receiver_index))->receiver_protocols = CRED_PROTOCOL_NOT_USED; /*!<  receiver protocols : Voice (GSM), SMS,MMS,E-mail,APP Push Notification : (A8A7A6A5A4A3A2A1: 0/1) */
	((alarm_service_receiver_t *)(receiver_global_ptr + receiver_index))->receiver_arc_protocols = CRED_ARC_PROTOCOL_NOT_USED; /*!<  receiver arc protocols : SIA IP,2 SIA Av 01,SIA SMS,Contact ID IP,Contact ID Voice channel,Contact ID SMS : (A8A7A6A5A4A3A2A1: 0/1)*/
	((alarm_service_receiver_t *)(receiver_global_ptr + receiver_index))->receiver_email_primary_channel = CRED_TCIP_IP_CHANNEL;	/*!<  Receiver : Email primary channel : 3G  or TCP-IP */
	((alarm_service_receiver_t *)(receiver_global_ptr + receiver_index))->receiver_app_primary_channel = CRED_TCIP_IP_CHANNEL;	/*!<  Receiver : App primary channel : 3G  or TCP-IP*/
	((alarm_service_receiver_t *)(receiver_global_ptr + receiver_index))->receiver_arc_primary_channel = CRED_TCIP_IP_CHANNEL;	/*!<  Receiver : Email primary channel : 3G  or TCP-IP*/
	((alarm_service_receiver_t *)(receiver_global_ptr + receiver_index))->receiver_arc_account_number=0;	/*!<  Receiver : arc accout number*/
	((alarm_service_receiver_t *)(receiver_global_ptr + receiver_index))->receiver_arc_ip_id=123456;	/*!<  Receiver : arc id number*/
	((alarm_service_receiver_t *)(receiver_global_ptr + receiver_index))->receiver_backup_enable = CRED_DISABLE;	/*!<  Receiver : enabling a backup channel for Email, App and ARC 0 or 1*/
	((alarm_service_receiver_t *)(receiver_global_ptr + receiver_index))->receiver_2way_audio_enable = CRED_DISABLE;	/*!<  Receiver : Two Way Audio Enable 0 or 1*/
	memset(((alarm_service_receiver_t *)(receiver_global_ptr + receiver_index))->receiver_description,0,CRED_RECEIVER_DESCRIPTION_STR);/*!< User description. */
	sprintf(((alarm_service_receiver_t *)(receiver_global_ptr + receiver_index))->receiver_description,"RECEIVER %02d",receiver_index);
	memset(((alarm_service_receiver_t *)(receiver_global_ptr + receiver_index))->receiver_phone_number,0,MAX_PHONE_NUMBER_STR);/*!< Receiver phone number */
	memset(((alarm_service_receiver_t *)(receiver_global_ptr + receiver_index))->receiver_sms_number,0,MAX_PHONE_NUMBER_STR);/*!< Receiver SMS number */
	memset(((alarm_service_receiver_t *)(receiver_global_ptr + receiver_index))->receiver_email_address,0,MAX_EMAIL_ADDRESS_STR);/*!< Receiver Email address */
	memset(((alarm_service_receiver_t *)(receiver_global_ptr + receiver_index))->receiver_ip_address,0,MAX_IP_ADDRESS_STR);/*!< Receiver IP address */
	((alarm_service_receiver_t *)(receiver_global_ptr + receiver_index))->receiver_ip_port = 0; /*!< Receiver IP Port */
	
	/*Open Receiver Code file:*/
	fp_receiver = fopen(CRED_RECEIVER_FILE_PATH, "r+");
	
	if(fp_receiver == NULL)
	{
		ALARM_TRACE_ERROR("RECIVER FILE ERROR : %s \n" CRED_RECEIVER_FILE_PATH);
	}
	else
	{
		/*File Exist */
		fseek(fp_receiver, (sizeof(alarm_service_receiver_t)*receiver_index), SEEK_SET);
		writebyte = fwrite(((alarm_service_receiver_t *)(receiver_global_ptr + receiver_index)), (sizeof(alarm_service_receiver_t)), 1, fp_receiver);
		ALARM_TRACE_INFO("Read Receiver %d Saved  Information \n ",receiver_index);
	}
	
	/*Close Receiver Code file*/
	fclose(fp_receiver);
	return err;
}
/**
 * \fn CRED_MW_Errors_t get_free_receiver_index(uint32_t *receiver_index_app)
 * \brief save one receiver structure to global str and NAND flash.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t get_free_receiver_index(uint32_t *receiver_index_app)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	int i;
	int receiver_index=0;
	for(i=1;i<=MAX_RECEIVER_NUMBER;i++)
	{	
			if(((alarm_service_receiver_t *)(receiver_global_ptr + i))->receiver_status == CRED_DISABLE)	/*!<  receiver : satatus (1: enabled CRED_DISABLE : disabled)*/
			{
				receiver_index = i;
				i = MAX_RECEIVER_NUMBER +1;
				break;	
			}
	}
	*receiver_index_app=receiver_index;
	return CRED_MW_NO_ERROR;
}


/**
 * \fn CRED_MW_Errors_t get_receiver_information(uint8_t receiver_index_app, uint32_t *receiver_ptr)
 * \brief get RF zone  informations.
 *
 * \param zone_index: zone index.
 * \return No parameter.
 */

CRED_MW_Errors_t get_receiver_information(uint8_t receiver_index_app, uint32_t *receiver_ptr)
{

	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	alarm_service_receiver_t *receiver_tmp_ptr=NULL;

	if(receiver_index_app > MAX_RECEIVER_NUMBER || receiver_index_app < 1)
	{
		ALARM_TRACE_ERROR("unsupported receiver index : %d \n",receiver_index_app);
		*receiver_ptr = receiver_tmp_ptr;
		return CRED_MW_NO_ERROR;
	}

	receiver_tmp_ptr = ((alarm_service_receiver_t *)(receiver_global_ptr + receiver_index_app));

	*receiver_ptr = receiver_tmp_ptr;

	return err;
}

/**
 * \fn CRED_MW_Errors_t load_cctv_information(void)
 * \brief load cctv global structure from NAND flash.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t load_cctv_information(void)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	int i;
	FILE *fp_cctv=NULL;
	alarm_service_cctv_t *cctv_ptr;
	/*Open cctv Code file:*/
	fp_cctv = fopen(CRED_CCTV_FILE_PATH, "r");
	if(fp_cctv == NULL)
	{
		/*File Does Not Exit => Create the File*/
		fp_cctv = fopen(CRED_CCTV_FILE_PATH, "w+");

		for(i=0;i<=MAX_CCTV_NUMBER;i++)
		{
			((alarm_service_cctv_t *)(cctv_global_ptr + i))->cctv_index = i;	/*!<  cctv : index (number 1 : 64)*/
			((alarm_service_cctv_t *)(cctv_global_ptr + i))->cctv_status = CRED_DISABLE;	/*!<  cctv : satatus (1: enabled 0 : disabled)*/
			((alarm_service_cctv_t *)(cctv_global_ptr + i))->cctv_fps_main = MAX_CCTV_FPS;      /*!<  Frame rate main stream*/
			((alarm_service_cctv_t *)(cctv_global_ptr + i))->cctv_fps_sub = MAX_CCTV_FPS;      /*!<  Frame rate sub stream*/
			((alarm_service_cctv_t *)(cctv_global_ptr + i))->cctv_image_quality_main = MAX_CCTV_IMAGE_QUALITY;      /*!< Image quality main stream*/
			((alarm_service_cctv_t *)(cctv_global_ptr + i))->cctv_image_quality_sub = MAX_CCTV_IMAGE_QUALITY;      /*!< Image quality sub stream*/
			((alarm_service_cctv_t *)(cctv_global_ptr + i))->cctv_ip_port = 0; /*!< cctv IP Port */
			memset(((alarm_service_cctv_t *)(cctv_global_ptr + i))->cctv_ip_address,0,MAX_IP_ADDRESS_STR);/*!< cctv IP address */
			memset(((alarm_service_cctv_t *)(cctv_global_ptr + i))->cctv_description,0,CRED_CCTV_DESCRIPTION_STR);/*!< cctv description. */
			memset(((alarm_service_cctv_t *)(cctv_global_ptr + i))->cctv_user,0,CRED_CCTV_DESCRIPTION_STR);/*!< cctv user. */
			memset(((alarm_service_cctv_t *)(cctv_global_ptr + i))->cctv_pwd,0,CRED_CCTV_DESCRIPTION_STR);/*!< cctv password. */
		}
		fwrite(cctv_global_ptr, (sizeof(alarm_service_cctv_t)*(MAX_CCTV_NUMBER+1)), 1, fp_cctv);
		ALARM_TRACE_INFO("Create cctv File for saving cctv information \n ");
	}
	else
	{
		/*File Exist Read Old Information*/
		fread(cctv_global_ptr, (sizeof(alarm_service_cctv_t)*(MAX_CCTV_NUMBER+1)), 1, fp_cctv);
		ALARM_TRACE_INFO("Read cctv Saved  Information \n ");
	}
	/*Close cctv Code file*/
	fclose(fp_cctv);
	return err;
}
/**
 * \fn CRED_MW_Errors_t alarm_factory_reset_cctv(void)
 * \brief Factory reset cctv.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_factory_reset_cctv(void)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	int i;
	FILE *fp_cctv=NULL;
	alarm_service_cctv_t *cctv_ptr;
	/*Open cctv Code file:*/
	fp_cctv = fopen(CRED_CCTV_FILE_PATH, "r+");
	if(fp_cctv == NULL)
	{
		/*File Does Not Exit => Create the File*/
		fp_cctv = fopen(CRED_CCTV_FILE_PATH, "w+");
	}
	for(i=1;i<=MAX_CCTV_NUMBER;i++)
	{
			((alarm_service_cctv_t *)(cctv_global_ptr + i))->cctv_index = i;	/*!<  cctv : index (number 1 : 64)*/
			((alarm_service_cctv_t *)(cctv_global_ptr + i))->cctv_status = CRED_DISABLE;	/*!<  cctv : satatus (1: enabled 0 : disabled)*/
			((alarm_service_cctv_t *)(cctv_global_ptr + i))->cctv_fps_main = MAX_CCTV_FPS;      /*!<  Frame rate main stream*/
			((alarm_service_cctv_t *)(cctv_global_ptr + i))->cctv_fps_sub = MAX_CCTV_FPS;      /*!<  Frame rate sub stream*/
			((alarm_service_cctv_t *)(cctv_global_ptr + i))->cctv_image_quality_main = MAX_CCTV_IMAGE_QUALITY;      /*!< Image quality main stream*/
			((alarm_service_cctv_t *)(cctv_global_ptr + i))->cctv_image_quality_sub = MAX_CCTV_IMAGE_QUALITY;      /*!< Image quality sub stream*/
			((alarm_service_cctv_t *)(cctv_global_ptr + i))->cctv_ip_port = 0; /*!< cctv IP Port */
			memset(((alarm_service_cctv_t *)(cctv_global_ptr + i))->cctv_ip_address,0,MAX_IP_ADDRESS_STR);/*!< cctv IP address */
			memset(((alarm_service_cctv_t *)(cctv_global_ptr + i))->cctv_description,0,CRED_CCTV_DESCRIPTION_STR);/*!< cctv description. */
			memset(((alarm_service_cctv_t *)(cctv_global_ptr + i))->cctv_user,0,CRED_CCTV_DESCRIPTION_STR);/*!< cctv user. */
			memset(((alarm_service_cctv_t *)(cctv_global_ptr + i))->cctv_pwd,0,CRED_CCTV_DESCRIPTION_STR);/*!< cctv password. */
	}
	fwrite(cctv_global_ptr, (sizeof(alarm_service_cctv_t)*(MAX_CCTV_NUMBER+1)), 1, fp_cctv);
	/*Close cctv Code file*/
	fclose(fp_cctv);
	return err;
}

/**
 * \fn CRED_MW_Errors_t save_cctv_information(uint8_t cctv_index,alarm_service_cctv_t *cctv_tmp_ptr)
 * \brief save one cctv structure to global str and NAND flash.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t save_cctv_information(uint32_t *cctv_index_app,alarm_service_cctv_t *cctv_tmp_ptr)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	int i;
	FILE *fp_cctv=NULL;
	int writebyte = 0;
	int cctv_index;
	alarm_service_cctv_t *cctv_ptr;
	cctv_index = *cctv_index_app;
	if(cctv_index > MAX_CCTV_NUMBER || cctv_index < 1)
	{
		ALARM_TRACE_ERROR("unsupported cctv index : %d \n",cctv_index);
		*cctv_index_app=0;
		return CRED_MW_NO_ERROR;
	}
	if(((alarm_service_cctv_t *)(cctv_global_ptr + cctv_index))->cctv_status == CRED_ENABBLE)
	{
		ALARM_TRACE_ERROR("cctv index used: %d \n",cctv_index);
		*cctv_index_app=0;
		return CRED_MW_NO_ERROR;
	}
	/*Open cctv Code file:*/
	fp_cctv = fopen(CRED_CCTV_FILE_PATH, "r+");
	if(fp_cctv == NULL)
	{
		ALARM_TRACE_ERROR("RECIVER FILE ERROR : %s \n" CRED_CCTV_FILE_PATH);
	}
	else
	{
		/*File Exist */
		fseek(fp_cctv, (sizeof(alarm_service_cctv_t)*cctv_index), SEEK_SET);
		memcpy(((alarm_service_cctv_t *)(cctv_global_ptr + cctv_index)),cctv_tmp_ptr,sizeof(alarm_service_cctv_t));
		((alarm_service_cctv_t *)(cctv_global_ptr + cctv_index))->cctv_index = cctv_index;
		((alarm_service_cctv_t *)(cctv_global_ptr + cctv_index))->cctv_status = CRED_ENABBLE;
		writebyte = fwrite(((alarm_service_cctv_t *)(cctv_global_ptr + cctv_index)), (sizeof(alarm_service_cctv_t)), 1, fp_cctv);
	}
	/*Close cctv Code file*/
	fclose(fp_cctv);
	return err;
}

/**
 * \fn CRED_MW_Errors_t get_free_cctv_index(uint32_t *cctv_index_app)
 * \brief save one cctv structure to global str and NAND flash.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t get_free_cctv_index(uint32_t *cctv_index_app)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	int i;
	int cctv_index=0;
	for(i=1;i<=MAX_CCTV_NUMBER;i++)
	{
			if(((alarm_service_cctv_t *)(cctv_global_ptr + i))->cctv_status == CRED_DISABLE)	/*!<  cctv : satatus (1: enabled CRED_DISABLE : disabled)*/
			{
				cctv_index = i;
				i = MAX_CCTV_NUMBER +1;
				break;
			}
	}
	*cctv_index_app=cctv_index;
	return CRED_MW_NO_ERROR;
}

/**
 * \fn CRED_MW_Errors_t get_cctv_information(uint8_t cctv_index_app, uint32_t *cctv_ptr)
 * \brief get RF zone  informations.
 *
 * \param zone_index: zone index.
 * \return No parameter.
 */

CRED_MW_Errors_t get_cctv_information(uint8_t cctv_index_app, uint32_t *cctv_ptr)
{

	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	alarm_service_cctv_t *cctv_tmp_ptr=NULL;
	if(cctv_index_app > MAX_CCTV_NUMBER || cctv_index_app < 1)
	{
		ALARM_TRACE_ERROR("unsupported cctv index : %d \n",cctv_index_app);
		*cctv_ptr = cctv_tmp_ptr;
		return CRED_MW_NO_ERROR;
	}
	cctv_tmp_ptr = ((alarm_service_cctv_t *)(cctv_global_ptr + cctv_index_app));
	*cctv_ptr = cctv_tmp_ptr;
	return err;
}
/**
 * \fn CRED_MW_Errors_t delete_cctv_information(uint8_t cctv_index)
 * \brief delete and save one cctv structure to global str and NAND flash.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t delete_cctv_information(uint32_t *cctv_index_app)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	int i;
	FILE *fp_cctv=NULL;
	int writebyte = 0;
	int cctv_index;
	cctv_index = *cctv_index_app;
	if(cctv_index > MAX_CCTV_NUMBER || cctv_index < 1)
	{
		ALARM_TRACE_ERROR("unsupported cctv index : %d \n",cctv_index);
		*cctv_index_app = 0;
		return CRED_MW_NO_ERROR;
	}
	((alarm_service_cctv_t *)(cctv_global_ptr + cctv_index))->cctv_index = cctv_index;	/*!<  cctv : index (number 1 : 64)*/
	((alarm_service_cctv_t *)(cctv_global_ptr + cctv_index))->cctv_status = CRED_DISABLE;	/*!<  cctv : satatus (1: enabled 0 : disabled)*/
	((alarm_service_cctv_t *)(cctv_global_ptr + cctv_index))->cctv_fps_main = MAX_CCTV_FPS;      /*!<  Frame rate main stream*/
	((alarm_service_cctv_t *)(cctv_global_ptr + cctv_index))->cctv_fps_sub = MAX_CCTV_FPS;      /*!<  Frame rate sub stream*/
	((alarm_service_cctv_t *)(cctv_global_ptr + cctv_index))->cctv_image_quality_main = MAX_CCTV_IMAGE_QUALITY;      /*!< Image quality main stream*/
	((alarm_service_cctv_t *)(cctv_global_ptr + cctv_index))->cctv_image_quality_sub = MAX_CCTV_IMAGE_QUALITY;      /*!< Image quality sub stream*/
	((alarm_service_cctv_t *)(cctv_global_ptr + cctv_index))->cctv_ip_port = 0; /*!< cctv IP Port */
	memset(((alarm_service_cctv_t *)(cctv_global_ptr + cctv_index))->cctv_ip_address,0,MAX_IP_ADDRESS_STR);/*!< cctv IP address */
	memset(((alarm_service_cctv_t *)(cctv_global_ptr + cctv_index))->cctv_description,0,CRED_CCTV_DESCRIPTION_STR);/*!< cctv description. */
	memset(((alarm_service_cctv_t *)(cctv_global_ptr + cctv_index))->cctv_user,0,CRED_CCTV_DESCRIPTION_STR);/*!< cctv user. */
	memset(((alarm_service_cctv_t *)(cctv_global_ptr + cctv_index))->cctv_pwd,0,CRED_CCTV_DESCRIPTION_STR);/*!< cctv password. */
	/*Open cctv Code file:*/
	fp_cctv = fopen(CRED_CCTV_FILE_PATH, "r+");
	if(fp_cctv == NULL)
	{
		ALARM_TRACE_ERROR("RECIVER FILE ERROR : %s \n" CRED_CCTV_FILE_PATH);
	}
	else
	{
		/*File Exist */
		fseek(fp_cctv, (sizeof(alarm_service_cctv_t)*cctv_index), SEEK_SET);
		writebyte = fwrite(((alarm_service_cctv_t *)(cctv_global_ptr + cctv_index)), (sizeof(alarm_service_cctv_t)), 1, fp_cctv);
		ALARM_TRACE_INFO("Read cctv %d Saved  Information \n ",cctv_index);
	}
	/*Close cctv Code file*/
	fclose(fp_cctv);
	return err;
}
/**
 * \fn CRED_MW_Errors_t load_area_information(void)
 * \brief load area global structure from NAND flash.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t load_area_information(void)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	int i;
	FILE *fp_area=NULL;
	alarm_service_area_t *area_ptr;
	/*Open area Code file:*/
	fp_area = fopen(CRED_AREA_FILE_PATH, "r");
	if(fp_area == NULL)
	{
		/*File Does Not Exit => Create the File*/
		fp_area = fopen(CRED_AREA_FILE_PATH, "w+");
		for(i=0;i<=MAX_AREA_NUMBER;i++)
		{
			((alarm_service_area_t *)(area_global_ptr + i))->area_index = i;	/*!<  area : index (number 1 : 4/8)*/
			((alarm_service_area_t *)(area_global_ptr + i))->area_arm_mode_status = CRED_ARM_MODE_DISARM;	/*!<  area : satatus (1: enabled 0 : disabled)*/
			if(i == 1) /*!<  Area 1 : by default enabled)*/
			{
				((alarm_service_area_t *)(area_global_ptr + i))->area_zone_32_1 = 0xFFFFFFFF;	/*!<  zone (1-32) :assigning to area*/
				((alarm_service_area_t *)(area_global_ptr + i))->area_zone_32_2 = 0xFFFFFFFF;	/*!<  zone (33-64): sassigning to area*/
				((alarm_service_area_t *)(area_global_ptr + i))->area_zone_include_32_1 = 0xFFFFFFFF;	/*!<  zone (1-32) : satatus toggling zones between INCLUDED 1 /EXCLUDED 0*/
				((alarm_service_area_t *)(area_global_ptr + i))->area_zone_include_32_2 = 0xFFFFFFFF;	/*!<  zone (33-64): satatus toggling zones between INCLUDED 1 /EXCLUDED 0*/
				((alarm_service_area_t *)(area_global_ptr + i))->area_zone_activate_32_1  = 0;	/*!<  zone (1-32) : satatus toggling zones between Activate 1 /Isolate 0*/
				((alarm_service_area_t *)(area_global_ptr + i))->area_zone_activate_32_2  = 0;	/*!<  zone (33-64): satatus toggling zones between Activate 1 /Isolate 0*/
			}else
			{
				((alarm_service_area_t *)(area_global_ptr + i))->area_zone_32_1 = 0;	/*!<  zone (1-32) :assigning to area*/
				((alarm_service_area_t *)(area_global_ptr + i))->area_zone_32_2 = 0;	/*!<  zone (33-64): sassigning to area*/
				((alarm_service_area_t *)(area_global_ptr + i))->area_zone_include_32_1 = 0;	/*!<  zone (1-32) : satatus toggling zones between INCLUDED 1 /EXCLUDED 0*/
				((alarm_service_area_t *)(area_global_ptr + i))->area_zone_include_32_2 = 0;	/*!<  zone (33-64): satatus toggling zones between INCLUDED 1 /EXCLUDED 0*/
				((alarm_service_area_t *)(area_global_ptr + i))->area_zone_activate_32_1  = 0;	/*!<  zone (1-32) : satatus toggling zones between Activate 1 /Isolate 0*/
				((alarm_service_area_t *)(area_global_ptr + i))->area_zone_activate_32_2  = 0;	/*!<  zone (33-64): satatus toggling zones between Activate 1 /Isolate 0*/

			}
			((alarm_service_area_t *)(area_global_ptr + i))->area_zone_rearm_option = CRED_REARM_OPTION_ALWAYS_REARM;	/*!<  Rearm Option: Default: Always Rearm */
			((alarm_service_area_t *)(area_global_ptr + i))->area_zone_rearm_number = 0;	/*!<  Rearm Option*/
			memset(((alarm_service_area_t *)(area_global_ptr + i))->area_description,0,CRED_AREA_DESCRIPTION_STR);/*!< area description. */
			sprintf(((alarm_service_area_t *)(area_global_ptr + i))->area_description,"Area %d",i);
			memset(((alarm_service_area_t *)(area_global_ptr + i))->area_p1_description,0,CRED_AREA_DESCRIPTION_STR);/*!< area description. */
			sprintf(((alarm_service_area_t *)(area_global_ptr + i))->area_p1_description,"Area %d partition 1",i);
			memset(((alarm_service_area_t *)(area_global_ptr + i))->area_p2_description,0,CRED_AREA_DESCRIPTION_STR);/*!< area description. */
			sprintf(((alarm_service_area_t *)(area_global_ptr + i))->area_p2_description,"Area %d partition 2",i);
		}
		fwrite(area_global_ptr, (sizeof(alarm_service_area_t)*(MAX_AREA_NUMBER+1)), 1, fp_area);
		ALARM_TRACE_INFO("Create area File for saving area information \n ");
	}
	else
	{
		/*File Exist Read Old Information*/
		fread(area_global_ptr, (sizeof(alarm_service_area_t)*(MAX_AREA_NUMBER+1)), 1, fp_area);
		ALARM_TRACE_INFO("Read area Saved  Information \n ");
	}
	/*Close area Code file*/
	fclose(fp_area);
	return err;
}
/**
 * \fn CRED_MW_Errors_t alarm_factory_reset_area(void)
 * \brief Factory reset area.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_factory_reset_area(void)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	int i;
	FILE *fp_area=NULL;
	alarm_service_area_t *area_ptr;
	/*Open area Code file:*/
	fp_area = fopen(CRED_AREA_FILE_PATH, "r+");
	if(fp_area == NULL)
	{
		/*File Does Not Exit => Create the File*/
		fp_area = fopen(CRED_AREA_FILE_PATH, "w+");
	}
	for(i=1;i<=MAX_AREA_NUMBER;i++)
	{
			((alarm_service_area_t *)(area_global_ptr + i))->area_index = i;	/*!<  area : index (number 1 : 4/8)*/
			((alarm_service_area_t *)(area_global_ptr + i))->area_arm_mode_status = CRED_ARM_MODE_DISARM;	/*!<  area : satatus (1: enabled 0 : disabled)*/
			if(i == 1) /*!<  Area 1 : by default enabled)*/
			{
				((alarm_service_area_t *)(area_global_ptr + i))->area_zone_32_1 = 0xFFFFFFFF;	/*!<  zone (1-32) :assigning to area*/
				((alarm_service_area_t *)(area_global_ptr + i))->area_zone_32_2 = 0xFFFFFFFF;	/*!<  zone (33-64): sassigning to area*/
				((alarm_service_area_t *)(area_global_ptr + i))->area_zone_include_32_1 = 0xFFFFFFFF;	/*!<  zone (1-32) : satatus toggling zones between INCLUDED 1 /EXCLUDED 0*/
				((alarm_service_area_t *)(area_global_ptr + i))->area_zone_include_32_2 = 0xFFFFFFFF;	/*!<  zone (33-64): satatus toggling zones between INCLUDED 1 /EXCLUDED 0*/
				((alarm_service_area_t *)(area_global_ptr + i))->area_zone_activate_32_1  = 0;	/*!<  zone (1-32) : satatus toggling zones between Activate 1 /Isolate 0*/
				((alarm_service_area_t *)(area_global_ptr + i))->area_zone_activate_32_2  = 0;	/*!<  zone (33-64): satatus toggling zones between Activate 1 /Isolate 0*/
			}else
			{
				((alarm_service_area_t *)(area_global_ptr + i))->area_zone_32_1 = 0;	/*!<  zone (1-32) :assigning to area*/
				((alarm_service_area_t *)(area_global_ptr + i))->area_zone_32_2 = 0;	/*!<  zone (33-64): sassigning to area*/
				((alarm_service_area_t *)(area_global_ptr + i))->area_zone_include_32_1 = 0;	/*!<  zone (1-32) : satatus toggling zones between INCLUDED 1 /EXCLUDED 0*/
				((alarm_service_area_t *)(area_global_ptr + i))->area_zone_include_32_2 = 0;	/*!<  zone (33-64): satatus toggling zones between INCLUDED 1 /EXCLUDED 0*/
				((alarm_service_area_t *)(area_global_ptr + i))->area_zone_activate_32_1  = 0;	/*!<  zone (1-32) : satatus toggling zones between Activate 1 /Isolate 0*/
				((alarm_service_area_t *)(area_global_ptr + i))->area_zone_activate_32_2  = 0;	/*!<  zone (33-64): satatus toggling zones between Activate 1 /Isolate 0*/
			}
			((alarm_service_area_t *)(area_global_ptr + i))->area_zone_rearm_option = CRED_REARM_OPTION_ALWAYS_REARM;	/*!<  Rearm Option: Default: Always Rearm */
			((alarm_service_area_t *)(area_global_ptr + i))->area_zone_rearm_number = 0;	/*!<  Rearm Option*/
			memset(((alarm_service_area_t *)(area_global_ptr + i))->area_description,0,CRED_AREA_DESCRIPTION_STR);/*!< area description. */
			sprintf(((alarm_service_area_t *)(area_global_ptr + i))->area_description,"Area %d",i);
			memset(((alarm_service_area_t *)(area_global_ptr + i))->area_p1_description,0,CRED_AREA_DESCRIPTION_STR);/*!< area description. */
			sprintf(((alarm_service_area_t *)(area_global_ptr + i))->area_p1_description,"Area %d partition 1",i);
			memset(((alarm_service_area_t *)(area_global_ptr + i))->area_p2_description,0,CRED_AREA_DESCRIPTION_STR);/*!< area description. */
			sprintf(((alarm_service_area_t *)(area_global_ptr + i))->area_p2_description,"Area %d partition 2",i);
	}
	fwrite(area_global_ptr, (sizeof(alarm_service_area_t)*(MAX_AREA_NUMBER+1)), 1, fp_area);
	/*Close area Code file*/
	fclose(fp_area);
	return err;
}
/**
 * \fn CRED_MW_Errors_t save_area_information(uint8_t area_index,alarm_service_area_t *area_tmp_ptr)
 * \brief save one area structure to global str and NAND flash.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t save_area_information(uint32_t *area_index_app,alarm_service_area_t *area_tmp_ptr)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	int i;
	FILE *fp_area=NULL;
	int writebyte = 0;
	int area_index;
	alarm_service_area_t *area_ptr;
	area_index = *area_index_app;
	if(area_index > MAX_AREA_NUMBER || area_index < 1)
	{
		ALARM_TRACE_ERROR("unsupported area index : %d \n",area_index);
		*area_index_app=0;
		return CRED_MW_NO_ERROR;
	}
	/*Open area Code file:*/
	fp_area = fopen(CRED_AREA_FILE_PATH, "r+");
	if(fp_area == NULL)
	{
		ALARM_TRACE_ERROR("RECIVER FILE ERROR : %s \n" CRED_AREA_FILE_PATH);
	}
	else
	{
		/*File Exist */
		fseek(fp_area, (sizeof(alarm_service_area_t)*area_index), SEEK_SET);
		memcpy(((alarm_service_area_t *)(area_global_ptr + area_index)),area_tmp_ptr,sizeof(alarm_service_area_t));
		((alarm_service_area_t *)(area_global_ptr + area_index))->area_index = area_index;
		writebyte = fwrite(((alarm_service_area_t *)(area_global_ptr + area_index)), (sizeof(alarm_service_area_t)), 1, fp_area);
	}
	/*Close area Code file*/
	fclose(fp_area);
	return err;
}
/**
 * \fn CRED_MW_Errors_t CRED_MW_Errors_t alarm_service_save_area_to_file(uint8_t area_index_app)
 * \brief copy one area structure to NAND flash.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_save_area_to_file(uint8_t area_index_app)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	int i;
	FILE *fp_area=NULL;
	int writebyte = 0;
	int area_index;
	alarm_service_area_t *area_ptr;
	if(area_index_app > MAX_AREA_NUMBER || area_index_app < 1)
	{
		ALARM_TRACE_ERROR("unsupported area index : %d \n",area_index_app);
		return CRED_MW_NO_ERROR;
	}
	/*Open area Code file:*/
	fp_area = fopen(CRED_AREA_FILE_PATH, "r+");
	if(fp_area == NULL)
	{
		ALARM_TRACE_ERROR("RECIVER FILE ERROR : %s \n" CRED_AREA_FILE_PATH);
	}
	else
	{
		/*File Exist */
		fseek(fp_area, (sizeof(alarm_service_area_t)*area_index_app), SEEK_SET);
		writebyte = fwrite(((alarm_service_area_t *)(area_global_ptr + area_index_app)), (sizeof(alarm_service_area_t)), 1, fp_area);
	}
	/*Close area Code file*/
	fclose(fp_area);
	return err;
}
/**
 * \fn CRED_MW_Errors_t alarm_service_add_zone_to_area(uint8_t zone_index_app, uint8_t zone_area)
 * \brief add RF zone to informations.
 *
 * \param zone_index: zone index,zone_area info.
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_add_zone_to_area(uint8_t zone_index_app, uint8_t zone_area)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	uint8_t i =0;
	uint8_t status_mask = 0x01;
	uint32_t zone_mask = 1;
	for(i=1;i<=MAX_AREA_NUMBER;i++)
	{
		if(zone_area & status_mask)
		{
			zone_mask = 1;
			ALARM_TRACE_INFO("Zone : %d included to Area : %d \n",zone_index_app,i);
			if( zone_index_app > 32 )
			{
				zone_mask =  zone_mask << (zone_index_app-33);
				((alarm_service_area_t *)(area_global_ptr + i))->area_zone_32_2 |= zone_mask;
				((alarm_service_area_t *)(area_global_ptr + i))->area_zone_include_32_2 |= zone_mask;
				if(((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index_app))->zone_status & CRED_ZONE_STATUS_ACTIVATED )
				{
					((alarm_service_area_t *)(area_global_ptr + i))->area_zone_activate_32_2 |= zone_mask;
				}else
				{
					((alarm_service_area_t *)(area_global_ptr + i))->area_zone_activate_32_2 &= (~zone_mask);
				}

				if(((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index_app))->zone_status & CRED_ZONE_STATUS_INHIBITED_NO )
				{
					((alarm_service_area_t *)(area_global_ptr + i))->area_zone_inhibited_no_32_2 |= zone_mask;
				}else
				{
					((alarm_service_area_t *)(area_global_ptr + i))->area_zone_inhibited_no_32_2 &= (~zone_mask);
				}
			}else
			{
				zone_mask =  zone_mask << (zone_index_app-1);
				((alarm_service_area_t *)(area_global_ptr + i))->area_zone_32_1 |= zone_mask;
				((alarm_service_area_t *)(area_global_ptr + i))->area_zone_include_32_1 |= zone_mask;
				if(((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index_app))->zone_status & CRED_ZONE_STATUS_ACTIVATED )
				{
					((alarm_service_area_t *)(area_global_ptr + i))->area_zone_activate_32_1 |= zone_mask;
				}else
				{
					((alarm_service_area_t *)(area_global_ptr + i))->area_zone_activate_32_1 &= (~zone_mask);
				}
				if(((alarm_service_rf_zone_t *)(rf_zone_global_ptr + zone_index_app))->zone_status & CRED_ZONE_STATUS_INHIBITED_NO )
				{
					((alarm_service_area_t *)(area_global_ptr + i))->area_zone_inhibited_no_32_1 |= zone_mask;
				}else
				{
					((alarm_service_area_t *)(area_global_ptr + i))->area_zone_inhibited_no_32_1 &= (~zone_mask);
				}
			}
			alarm_save_area_to_file(i);
		}
		status_mask =  status_mask <<1;
	}
	return err;
}
/**
 * \fn CRED_MW_Errors_t alarm_delete_zone_from_area(uint8_t zone_index_app)
 * \brief add RF zone to informations.
 *
 * \param zone_index: zone index.
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_delete_zone_from_area(uint8_t zone_index_app)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	uint8_t i =0;
	uint32_t zone_mask = 1;
	for(i=1;i<=MAX_AREA_NUMBER;i++)
	{
		zone_mask = 1;
		ALARM_TRACE_INFO("Zone : %d excluded from Area : %d \n",zone_index_app,i);
		if( zone_index_app > 32 )
		{
			zone_mask =  zone_mask << (zone_index_app-33);
			((alarm_service_area_t *)(area_global_ptr + i))->area_zone_32_2 &= (~zone_mask);
			((alarm_service_area_t *)(area_global_ptr + i))->area_zone_include_32_2 &= (~zone_mask);
			((alarm_service_area_t *)(area_global_ptr + i))->area_zone_activate_32_2 |= zone_mask;
			((alarm_service_area_t *)(area_global_ptr + i))->area_zone_inhibited_no_32_2 &= (~zone_mask);
		}else
		{
			zone_mask =  zone_mask << (zone_index_app-1);
			((alarm_service_area_t *)(area_global_ptr + i))->area_zone_32_1 &= (~zone_mask);
			((alarm_service_area_t *)(area_global_ptr + i))->area_zone_include_32_1 &= (~zone_mask);
			((alarm_service_area_t *)(area_global_ptr + i))->area_zone_activate_32_1 |= zone_mask;
			((alarm_service_area_t *)(area_global_ptr + i))->area_zone_inhibited_no_32_1 &= (~zone_mask);
		}
		alarm_save_area_to_file(i);
	}
	return err;
}
/**
 * \fn CRED_MW_Errors_t get_area_information(uint8_t area_index_app, uint32_t *area_ptr)
 * \brief get RF zone  informations.
 *
 * \param zone_index: zone index.
 * \return No parameter.
 */

CRED_MW_Errors_t get_area_information(uint8_t area_index_app, uint32_t *area_ptr)
{

	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	alarm_service_area_t *area_tmp_ptr=NULL;
	if(area_index_app > MAX_AREA_NUMBER || area_index_app < 1)
	{
		ALARM_TRACE_ERROR("unsupported area index : %d \n",area_index_app);
		*area_ptr = area_tmp_ptr;
		return CRED_MW_NO_ERROR;
	}
	area_tmp_ptr = ((alarm_service_area_t *)(area_global_ptr + area_index_app));
	*area_ptr = area_tmp_ptr;
	return err;
}

/**
 * \fn CRED_MW_Errors_t CRED_MW_Errors_t reset_area_information(uint8_t area_index_app)
 * \brief reset and save one area structure to global str and NAND flash.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t reset_area_information(uint8_t area_index_app)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	int i;
	FILE *fp_area=NULL;
	int writebyte = 0;

	if(area_index_app > MAX_AREA_NUMBER || area_index_app < 1)
	{
		ALARM_TRACE_ERROR("unsupported area index : %d \n",area_index_app);
		return CRED_MW_NO_ERROR;
	}
	/*Open area Code file:*/
	fp_area = fopen(CRED_AREA_FILE_PATH, "r+");
	if(fp_area == NULL)
	{
		ALARM_TRACE_ERROR("RECIVER FILE ERROR : %s \n" CRED_AREA_FILE_PATH);
	}
	else
	{
		((alarm_service_area_t *)(area_global_ptr + area_index_app))->area_index = area_index_app;	/*!<  area : index (number 1 : 4/8)*/
		((alarm_service_area_t *)(area_global_ptr + area_index_app))->area_arm_mode_status = CRED_ARM_MODE_TOTAL_ARM;	/*!<  area : satatus (1: enabled 0 : disabled)*/
		if(i == 1)/*!<  Area 1 : by default enabled)*/
		{
			((alarm_service_area_t *)(area_global_ptr + area_index_app))->area_zone_32_1 = 0xFFFFFFFF;	/*!<  zone (1-32) :assigning to area*/
			((alarm_service_area_t *)(area_global_ptr + area_index_app))->area_zone_32_2 = 0xFFFFFFFF;	/*!<  zone (33-64): sassigning to area*/
			((alarm_service_area_t *)(area_global_ptr + area_index_app))->area_zone_include_32_1 = 0xFFFFFFFF;	/*!<  zone (1-32) : satatus toggling zones between INCLUDED 1 /EXCLUDED 0*/
			((alarm_service_area_t *)(area_global_ptr + area_index_app))->area_zone_include_32_2 = 0xFFFFFFFF;	/*!<  zone (33-64): satatus toggling zones between INCLUDED 1 /EXCLUDED 0*/
			((alarm_service_area_t *)(area_global_ptr + area_index_app))->area_zone_activate_32_1  = 0;	/*!<  zone (1-32) : satatus toggling zones between Activate 1 /Isolate 0*/
			((alarm_service_area_t *)(area_global_ptr + area_index_app))->area_zone_activate_32_2  = 0;	/*!<  zone (33-64): satatus toggling zones between Activate 1 /Isolate 0*/
		}else
		{
			((alarm_service_area_t *)(area_global_ptr + area_index_app))->area_zone_32_1 = 0;	/*!<  zone (1-32) :assigning to area*/
			((alarm_service_area_t *)(area_global_ptr + area_index_app))->area_zone_32_2 = 0;	/*!<  zone (33-64): sassigning to area*/
			((alarm_service_area_t *)(area_global_ptr + area_index_app))->area_zone_include_32_1 = 0;	/*!<  zone (1-32) : satatus toggling zones between INCLUDED 1 /EXCLUDED 0*/
			((alarm_service_area_t *)(area_global_ptr + area_index_app))->area_zone_include_32_2 = 0;	/*!<  zone (33-64): satatus toggling zones between INCLUDED 1 /EXCLUDED 0*/
			((alarm_service_area_t *)(area_global_ptr + area_index_app))->area_zone_activate_32_1  = 0;	/*!<  zone (1-32) : satatus toggling zones between Activate 1 /Isolate 0*/
			((alarm_service_area_t *)(area_global_ptr + area_index_app))->area_zone_activate_32_2  = 0;	/*!<  zone (33-64): satatus toggling zones between Activate 1 /Isolate 0*/
		}
		((alarm_service_area_t *)(area_global_ptr + area_index_app))->area_zone_rearm_option = CRED_REARM_OPTION_ALWAYS_REARM;	/*!<  Rearm Option: Default: Always Rearm */
		((alarm_service_area_t *)(area_global_ptr + area_index_app))->area_zone_rearm_number = 0;	/*!<  Rearm Option*/
		memset(((alarm_service_area_t *)(area_global_ptr + area_index_app))->area_description,0,CRED_AREA_DESCRIPTION_STR);/*!< area description. */
		sprintf(((alarm_service_area_t *)(area_global_ptr + area_index_app))->area_description,"Area %d",area_index_app);
		memset(((alarm_service_area_t *)(area_global_ptr + area_index_app))->area_p1_description,0,CRED_AREA_DESCRIPTION_STR);/*!< area description. */
		sprintf(((alarm_service_area_t *)(area_global_ptr + area_index_app))->area_p1_description,"Area %d partition 1",area_index_app);
		memset(((alarm_service_area_t *)(area_global_ptr + area_index_app))->area_p2_description,0,CRED_AREA_DESCRIPTION_STR);/*!< area description. */
		sprintf(((alarm_service_area_t *)(area_global_ptr + area_index_app))->area_p2_description,"Area %d partition 2",area_index_app);

		/*File Exist */
		fseek(fp_area, (sizeof(alarm_service_area_t)*area_index_app), SEEK_SET);
		writebyte = fwrite(((alarm_service_area_t *)(area_global_ptr + area_index_app)), (sizeof(alarm_service_area_t)), 1, fp_area);
	}
	/*Close area Code file*/
	fclose(fp_area);
	return err;
}
/**
 * \fn CRED_MW_Errors_t load_arm_option_information(void)
 * \brief load arm_option global structure from NAND flash.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t load_arm_option_information(void)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	int i;
	FILE *fp_arm_option=NULL;
	alarm_service_arm_option_t *arm_option_ptr;
	/*Open arm_option Code file:*/
	fp_arm_option = fopen(CRED_ARM_OPTION_FILE_PATH, "r");
	if(fp_arm_option == NULL)
	{
		/*File Does Not Exit => Create the File*/
		fp_arm_option = fopen(CRED_ARM_OPTION_FILE_PATH, "w+");

		((alarm_service_arm_option_t *)(arm_option_global_ptr))->forced_arm_option = CRED_FORCED_ARM_EXCLUDE;	/*!<  Forced Arm Option : Exclude (DEFAULT)*/
		((alarm_service_arm_option_t *)(arm_option_global_ptr))->check_exit_zone = CRED_OFF;	/*!<  Check Exit Zone: No (DEFAULT)*/
		((alarm_service_arm_option_t *)(arm_option_global_ptr))->entry_buzzer_arm = CRED_OFF;	/*!<  Entry Buzzer Arm: OFF (DEFAULT)*/
		((alarm_service_arm_option_t *)(arm_option_global_ptr))->exit_buzzer_arm = CRED_OFF;	/*!<  Exit Buzzer Arm: OFF (DEFAULT)*/
		((alarm_service_arm_option_t *)(arm_option_global_ptr))->entry_time_part_arm = CRED_ON;	/*!<  Entry Time Part arm: ON (DEFAULT)*/
		((alarm_service_arm_option_t *)(arm_option_global_ptr))->silent_entry_part_arm = CRED_OFF;	/*!<  Silent Entry Part Arm: OFF (DEFAULT)*/
		((alarm_service_arm_option_t *)(arm_option_global_ptr))->exit_fault_ext_bell = CRED_OFF;	/*!<  Exit Fault Ext bell: OFF (DEFAULT)*/
		((alarm_service_arm_option_t *)(arm_option_global_ptr))->quick_arm = CRED_ON;	/*!<  Quick Arm: ON (DEFAULT)*/
		((alarm_service_arm_option_t *)(arm_option_global_ptr))->alarm_cycles = 0;	/*!<  Alarm cycles: 0 (DEFAULT)*/
		((alarm_service_arm_option_t *)(arm_option_global_ptr))->fault_cycles = 0;	/*!<  Fault cycles: 0 (DEFAULT)*/
		((alarm_service_arm_option_t *)(arm_option_global_ptr))->disarm_stops_voice_call = CRED_ENABBLE;	/*!<  Disarm stops voice call: ENABLED (DEFAULT)*/
		((alarm_service_arm_option_t *)(arm_option_global_ptr))->disarm_images = CRED_ENABBLE;	/*!<  Disarm images: ENABLED (DEFAULT)*/
		((alarm_service_arm_option_t *)(arm_option_global_ptr))->mask_tamper  = CRED_DISABLE;	/*!<  Disarm images: DISABLED (DEFAULT)*/
		memcpy(arm_option_global_ptr+1,arm_option_global_ptr,sizeof(alarm_service_arm_option_t));
		fwrite(arm_option_global_ptr, (sizeof(alarm_service_arm_option_t))*2, 1, fp_arm_option);
		ALARM_TRACE_INFO("Create arm_option File for saving arm_option information \n ");
	}
	else
	{
		/*File Exist Read Old Information*/
		fread(arm_option_global_ptr, (sizeof(alarm_service_arm_option_t))*2, 1, fp_arm_option);
		ALARM_TRACE_INFO("Read arm_option Saved  Information \n ");
	}
	/*Close arm_option Code file*/
	fclose(fp_arm_option);
	return err;
}

/**
 * \fn CRED_MW_Errors_t reset_arm_option_information(void)
 * \brief rest arm_option global structure from NAND flash.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t reset_arm_option_information(void)
{
	CRED_MW_Errors_t err = CRED_MW_NO_ERROR;
	int i;
	FILE *fp_arm_option=NULL;
	alarm_service_arm_option_t *arm_option_ptr;
	/*Open arm_option Code file:*/
	fp_arm_option = fopen(CRED_ARM_OPTION_FILE_PATH, "r+");
	if(fp_arm_option == NULL)
	{
		/*File Does Not Exit => Create the File*/
		fp_arm_option = fopen(CRED_ARM_OPTION_FILE_PATH, "w+");
	}

	((alarm_service_arm_option_t *)(arm_option_global_ptr))->forced_arm_option = CRED_FORCED_ARM_EXCLUDE;	/*!<  Forced Arm Option : Exclude (DEFAULT)*/
	((alarm_service_arm_option_t *)(arm_option_global_ptr))->check_exit_zone = CRED_OFF;	/*!<  Check Exit Zone: No (DEFAULT)*/
	((alarm_service_arm_option_t *)(arm_option_global_ptr))->entry_buzzer_arm = CRED_OFF;	/*!<  Entry Buzzer Arm: OFF (DEFAULT)*/
	((alarm_service_arm_option_t *)(arm_option_global_ptr))->exit_buzzer_arm = CRED_OFF;	/*!<  Exit Buzzer Arm: OFF (DEFAULT)*/
	((alarm_service_arm_option_t *)(arm_option_global_ptr))->entry_time_part_arm = CRED_ON;	/*!<  Entry Time Part arm: ON (DEFAULT)*/
	((alarm_service_arm_option_t *)(arm_option_global_ptr))->silent_entry_part_arm = CRED_OFF;	/*!<  Silent Entry Part Arm: OFF (DEFAULT)*/
	((alarm_service_arm_option_t *)(arm_option_global_ptr))->exit_fault_ext_bell = CRED_OFF;	/*!<  Exit Fault Ext bell: OFF (DEFAULT)*/
	((alarm_service_arm_option_t *)(arm_option_global_ptr))->quick_arm = CRED_ON;	/*!<  Quick Arm: ON (DEFAULT)*/
	((alarm_service_arm_option_t *)(arm_option_global_ptr))->alarm_cycles = 0;	/*!<  Alarm cycles: 0 (DEFAULT)*/
	((alarm_service_arm_option_t *)(arm_option_global_ptr))->fault_cycles = 0;	/*!<  Fault cycles: 0 (DEFAULT)*/
	((alarm_service_arm_option_t *)(arm_option_global_ptr))->disarm_stops_voice_call = CRED_ENABBLE;	/*!<  Disarm stops voice call: ENABLED (DEFAULT)*/
	((alarm_service_arm_option_t *)(arm_option_global_ptr))->disarm_images = CRED_ENABBLE;	/*!<  Disarm images: ENABLED (DEFAULT)*/
	((alarm_service_arm_option_t *)(arm_option_global_ptr))->mask_tamper = CRED_DISABLE;	/*!<  Disarm images: ENABLED (DEFAULT)*/
	memcpy(arm_option_global_ptr+1,arm_option_global_ptr,sizeof(alarm_service_arm_option_t));
	fwrite(arm_option_global_ptr, (sizeof(alarm_service_arm_option_t))*2, 1, fp_arm_option);
	ALARM_TRACE_INFO("Create arm_option File for saving arm_option information \n ");

	/*Close arm_option Code file*/
	fclose(fp_arm_option);
	return err;
}
/**
 * \fn CRED_MW_Errors_t alarm_update_arm_option(uint32_t *arm_option_ptr)
 * \brief update arm option.
 *
 * \param : uint32_t *alarm_system_info_ptr
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_save_arm_option(alarm_service_arm_option_t *arm_option_ptr)
{
	FILE *fp_arm_option=NULL;
	if(arm_option_ptr == NULL)
	{
		ALARM_TRACE_ERROR("arm_option_ptr null pointer \n");
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	/*Open arm_option Code file:*/
	fp_arm_option = fopen(CRED_ARM_OPTION_FILE_PATH, "r+");
	if(fp_arm_option == NULL)
	{
		/*File Does Not Exit => Create the File*/
		fp_arm_option = fopen(CRED_ARM_OPTION_FILE_PATH, "w+");
	}

	/*!< back up copy setting */
	memcpy(arm_option_global_ptr,arm_option_ptr,sizeof(alarm_service_arm_option_t));
	memcpy(arm_option_global_ptr+1,arm_option_global_ptr,sizeof(alarm_service_arm_option_t));
	
	fwrite(arm_option_global_ptr, (sizeof(alarm_service_arm_option_t)*(2)), 1, fp_arm_option);

	/*Close sys info file*/	
	fclose(fp_arm_option);
	
	return CRED_MW_NO_ERROR;
}
/**
 * \fn CRED_MW_Errors_t alarm_get_arm_option(uint32_t *arm_option_ptr)
 * \brief get arm option.
 *
 * \param : uint32_t *alarm_system_info_ptr
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_get_arm_option(alarm_service_arm_option_t *arm_option_ptr)
{
	
	if(arm_option_ptr == NULL)
	{
		ALARM_TRACE_ERROR("arm_option_ptr null pointer \n");
		return CRED_MW_ERROR_BAD_PARAMETER;
	}

	/*!< back up copy setting */
	memcpy(arm_option_ptr,arm_option_global_ptr,sizeof(alarm_service_arm_option_t));

	
	return CRED_MW_NO_ERROR;
}
/**
 * \fn CRED_MW_Errors_t alarm_total_arm_system(void)
 * \brief Total Arm system.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_total_arm_system(void)
{
	alarm_arm_system(CRED_ARM_MODE_TOTAL_ARM);
	ALARM_TRACE_INFO("arm mode: %d  \n",arm_mode_global);
	return CRED_MW_NO_ERROR;
}
/**
 * \fn CRED_MW_Errors_t alarm_total_arm_no_dealy_system(void)
 * \brief Total Arm no delay system.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_total_arm_no_dealy_system(void)
{
	alarm_arm_system(CRED_ARM_MODE_TOTAL_ARM_NO_DELAY);
	ALARM_TRACE_INFO("arm mode: %d  \n",arm_mode_global);
	return CRED_MW_NO_ERROR;
}
/**
 * \fn CRED_MW_Errors_t alarm_p1_arm_system(void)
 * \brief p1 Arm system.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_p1_arm_system(void)
{
	alarm_arm_system(CRED_ARM_MODE_ARM_P1);
	ALARM_TRACE_INFO("arm mode: %d  \n",arm_mode_global);
	return CRED_MW_NO_ERROR;
}
/**
 * \fn CRED_MW_Errors_t alarm_p1_arm_no_dealy_system(void)
 * \brief p1 Arm no delay system.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_p1_arm_no_dealy_system(void)
{
	alarm_arm_system(CRED_ARM_MODE_ARM_P1_NO_DELAY);
	ALARM_TRACE_INFO("arm mode: %d  \n",arm_mode_global);
	return CRED_MW_NO_ERROR;
}
/**
 * \fn CRED_MW_Errors_t alarm_p2_arm_system(void)
 * \brief Total Arm system.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_p2_arm_system(void)
{
	alarm_arm_system(CRED_ARM_MODE_ARM_P2);
	ALARM_TRACE_INFO("arm mode: %d  \n",arm_mode_global);
	return CRED_MW_NO_ERROR;
}
/**
 * \fn CRED_MW_Errors_t alarm_p2_arm_no_dealy_system(void)
 * \brief p1 Arm no delay system.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_p2_arm_no_dealy_system(void)
{
	alarm_arm_system(CRED_ARM_MODE_ARM_P2_NO_DELAY);
	ALARM_TRACE_INFO("arm mode: %d  \n",arm_mode_global);
	return CRED_MW_NO_ERROR;
}
/**
 * \fn CRED_MW_Errors_t alarm_p12_arm_system(void)
 * \brief Total Arm system.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_p12_arm_system(void)
{
	alarm_arm_system(CRED_ARM_MODE_ARM_P12);
	ALARM_TRACE_INFO("arm mode: %d  \n",arm_mode_global);
	return CRED_MW_NO_ERROR;
}
/**
 * \fn CRED_MW_Errors_t alarm_p12_arm_no_dealy_system(void)
 * \brief p1 Arm no delay system.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_p12_arm_no_dealy_system(void)
{
	alarm_arm_system(CRED_ARM_MODE_ARM_P12_NO_DELAY);
	ALARM_TRACE_INFO("arm mode: %d  \n",arm_mode_global);
	return CRED_MW_NO_ERROR;
}
/**
 * \fn CRED_MW_Errors_t alarm_disarm_system(void)
 * \brief arm all system.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_disarm_system(void)
{
	alarm_arm_system(CRED_ARM_MODE_DISARM);
	ALARM_TRACE_INFO("arm mode: %d  \n",arm_mode_global);
	return CRED_MW_NO_ERROR;
}

/**
 * \fn CRED_MW_Errors_t alarm_arm_system(void)
 * \brief arm all system.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_arm_system(alarm_service_arm_mode_type_t partition_mode)
{
	int i =0;
	/*!<  Start Arm/Disarm Area */
	for(i=1;i<=MAX_AREA_NUMBER;i++)
	{
		ALARM_TRACE_INFO("Start Arming Area %d procedure to Arm mode %d\n",i,partition_mode);
		alarm_arm_area(partition_mode,i);
	}
	
	if(partition_mode == CRED_ARM_MODE_DISARM)
	{
		arm_mode_global = CRED_ARM_MODE_DISARM;
		if(CRED_MW_NO_ERROR != PCA9535_SetLedValue(CRED_LED_GREEN, 1))
		{
			ALARM_TRACE_ERROR("fail to set LED State: ON  \n");
			return CRED_MW_ERROR_BAD_PARAMETER;
		}
	}else
	{
		arm_mode_global |= partition_mode;
		if(CRED_MW_NO_ERROR != PCA9535_SetLedValue(CRED_LED_GREEN, 0))
		{
			ALARM_TRACE_ERROR("fail to set LED State: ON  \n");
			return CRED_MW_ERROR_BAD_PARAMETER;
		}
	}
	return CRED_MW_NO_ERROR;
}

/**
 * \fn CRED_MW_Errors_t alarm_total_arm_area(void)
 * \brief Total Arm system.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_total_arm_area(uint8_t area_index_app)
{
	alarm_arm_area(CRED_ARM_MODE_TOTAL_ARM,area_index_app);
	ALARM_TRACE_INFO("arm mode: %d  \n",arm_mode_global);
	return CRED_MW_NO_ERROR;
}
/**
 * \fn CRED_MW_Errors_t alarm_total_arm_no_dealy_area(void)
 * \brief Total Arm no delay system.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_total_arm_no_dealy_area(uint8_t area_index_app)
{
	alarm_arm_area(CRED_ARM_MODE_TOTAL_ARM_NO_DELAY,area_index_app);
	ALARM_TRACE_INFO("arm mode: %d  \n",arm_mode_global);
	return CRED_MW_NO_ERROR;
}
/**
 * \fn CRED_MW_Errors_t alarm_p1_arm_area(void)
 * \brief p1 Arm system.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_p1_arm_area(uint8_t area_index_app)
{
	alarm_arm_area(CRED_ARM_MODE_ARM_P1,area_index_app);
	ALARM_TRACE_INFO("arm mode: %d  \n",arm_mode_global);
	return CRED_MW_NO_ERROR;
}
/**
 * \fn CRED_MW_Errors_t alarm_p1_arm_no_dealy_area(void)
 * \brief p1 Arm no delay system.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_p1_arm_no_dealy_area(uint8_t area_index_app)
{
	alarm_arm_area(CRED_ARM_MODE_ARM_P1_NO_DELAY,area_index_app);
	ALARM_TRACE_INFO("arm mode: %d  \n",arm_mode_global);
	return CRED_MW_NO_ERROR;
}
/**
 * \fn CRED_MW_Errors_t alarm_p2_arm_area(void)
 * \brief Total Arm system.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_p2_arm_area(uint8_t area_index_app)
{
	alarm_arm_area(CRED_ARM_MODE_ARM_P2,area_index_app);
	ALARM_TRACE_INFO("arm mode: %d  \n",arm_mode_global);
	return CRED_MW_NO_ERROR;
}
/**
 * \fn CRED_MW_Errors_t alarm_p2_arm_no_dealy_area(void)
 * \brief p1 Arm no delay system.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_p2_arm_no_dealy_area(uint8_t area_index_app)
{
	alarm_arm_area(CRED_ARM_MODE_ARM_P2_NO_DELAY,area_index_app);
	ALARM_TRACE_INFO("arm mode: %d  \n",arm_mode_global);
	return CRED_MW_NO_ERROR;
}
/**
 * \fn CRED_MW_Errors_t alarm_p12_arm_area(void)
 * \brief Total Arm system.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_p12_arm_area(uint8_t area_index_app)
{
	alarm_arm_area(CRED_ARM_MODE_ARM_P12,area_index_app);
	ALARM_TRACE_INFO("arm mode: %d  \n",arm_mode_global);
	return CRED_MW_NO_ERROR;
}
/**
 * \fn CRED_MW_Errors_t alarm_p12_arm_no_dealy_area(void)
 * \brief p1 Arm no delay system.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_p12_arm_no_dealy_area(uint8_t area_index_app)
{
	alarm_arm_area(CRED_ARM_MODE_ARM_P12_NO_DELAY,area_index_app);
	ALARM_TRACE_INFO("arm mode: %d  \n",arm_mode_global);
	return CRED_MW_NO_ERROR;
}
/**
 * \fn CRED_MW_Errors_t alarm_disarm_area(void)
 * \brief arm all system.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_disarm_area(uint8_t area_index_app)
{
	alarm_arm_area(CRED_ARM_MODE_DISARM,area_index_app);
	ALARM_TRACE_INFO("arm mode: %d  \n",arm_mode_global);
	return CRED_MW_NO_ERROR;
}

/**
 * \fn CRED_MW_Errors_t alarm_arm_area(alarm_service_arm_mode_type_t partition_mode,uint8_t area_index)
 * \brief arm area.
 *
 * \param : alarm_service_arm_mode_type_t partition_mode,uint8_t area_index.
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_arm_area(alarm_service_arm_mode_type_t partition_mode,uint8_t area_index)
{
	int i =0;
	uint8_t delay_flag =0;
	uint8_t zone_partition_mode;
	uint8_t area_arm_mode_tmp;
	uint8_t start_timer_entry_1_flag=0;
	uint8_t start_timer_entry_2_flag=0;
	uint8_t start_timer_entry_door_flag=0;
	uint32_t zone_mask = 1;
	uint32_t area_zone_include_tmp;
	uint32_t area_zone_activate_tmp;
	uint32_t area_zone_inhibit_tmp;
	area_arm_mode_tmp = ((alarm_service_area_t *)(area_global_ptr + i))->area_arm_mode_status;
	/*!<  check zone status */
	/*!<  check forced arm option  */
	/*!<  Entry Buzzer Arm option  */
	/*!<  Exit Buzzer Arm option  */
	/*!<  Entry Time Part arm option  */
	/*!<  Silent Entry Part Arm option  */
	/*!<  Exit Fault Ext bell option  */
	ALARM_TRACE_INFO("partition_mode %d \n", partition_mode);
	switch(partition_mode)
	{
		case CRED_ARM_MODE_DISARM :
		{
			ALARM_TRACE_INFO("CRED_ARM_MODE_DISARM\n");
			area_arm_mode_tmp = CRED_ARM_MODE_DISARM;
			zone_partition_mode = 0;
		}
		break;
		case CRED_ARM_MODE_TOTAL_ARM :
		{
			ALARM_TRACE_INFO("CRED_ARM_MODE_TOTAL_ARM\n");
			area_arm_mode_tmp = CRED_ARM_MODE_TOTAL_ARM;
			delay_flag = 1;
			zone_partition_mode = CRED_ZONE_PARTITION_ALL;
		}
		break;
		case CRED_ARM_MODE_TOTAL_ARM_NO_DELAY :
		{
			ALARM_TRACE_INFO("CRED_ARM_MODE_TOTAL_ARM_NO_DELAY\n");
			area_arm_mode_tmp = CRED_ARM_MODE_TOTAL_ARM_NO_DELAY;
			delay_flag = 0;
			zone_partition_mode = CRED_ZONE_PARTITION_ALL;
		}
		break;
		case CRED_ARM_MODE_ARM_P1 :
		{
			ALARM_TRACE_INFO("CRED_ARM_MODE_ARM_P1\n");
			area_arm_mode_tmp |= CRED_ARM_MODE_ARM_P1;
			delay_flag = 1;
			zone_partition_mode = CRED_ZONE_PARTITION_1;
		}
		break;
		case CRED_ARM_MODE_ARM_P1_NO_DELAY :
		{
			ALARM_TRACE_INFO("CRED_ARM_MODE_ARM_P1_NO_DELAY\n");
			area_arm_mode_tmp |= CRED_ARM_MODE_ARM_P1_NO_DELAY;
			delay_flag = 0;
			zone_partition_mode = CRED_ZONE_PARTITION_1;
		}
		break;
		case CRED_ARM_MODE_ARM_P2 :
		{
			ALARM_TRACE_INFO("CRED_ARM_MODE_ARM_P2\n");
			area_arm_mode_tmp |= CRED_ARM_MODE_ARM_P2;
			delay_flag = 1;
			zone_partition_mode = CRED_ZONE_PARTITION_2;
		}
		break;
		case CRED_ARM_MODE_ARM_P2_NO_DELAY :
		{
			ALARM_TRACE_INFO("CRED_ARM_MODE_ARM_P2_NO_DELAY\n");
			area_arm_mode_tmp |= CRED_ARM_MODE_ARM_P2_NO_DELAY;
			delay_flag = 0;
			zone_partition_mode = CRED_ZONE_PARTITION_2;
		}
		break;
		case CRED_ARM_MODE_ARM_P12 :
		{
			ALARM_TRACE_INFO("CRED_ARM_MODE_ARM_P12\n");
			area_arm_mode_tmp |= CRED_ARM_MODE_ARM_P12;
			delay_flag = 1;
			zone_partition_mode = CRED_ZONE_PARTITION_1_2;
		}
		break;
		case CRED_ARM_MODE_ARM_P12_NO_DELAY :
		{
			ALARM_TRACE_INFO("CRED_ARM_MODE_ARM_P12_NO_DELAY\n");
			area_arm_mode_tmp |= CRED_ARM_MODE_ARM_P12_NO_DELAY;
			delay_flag = 0;
			zone_partition_mode = CRED_ZONE_PARTITION_1_2;
		}
		break;
		default:
			ALARM_TRACE_ERROR("Not supported Arm mode : %d \n",partition_mode);
		break;
	}
	/*!<  Setting of Area Arm mode */
	((alarm_service_area_t *)(area_global_ptr + area_index))->area_arm_mode_status = area_arm_mode_tmp;
	/*!<  Start Arm/Disarm Zone */
	for(i=1;i<=MAX_RF_ZONE_NUMBER;i++)
	{
		/*!<  check zone status: Included to the area or No */
		zone_mask = 1;
		if( i > 32 )
		{
			zone_mask =  zone_mask << (i-33);
			area_zone_include_tmp = ((alarm_service_area_t *)(area_global_ptr + area_index))->area_zone_include_32_2;
			/*area_zone_activate_tmp = ((alarm_service_area_t *)(area_global_ptr + area_index))->area_zone_activate_32_2;
			area_zone_inhibit_tmp = ((alarm_service_area_t *)(area_global_ptr + area_index))->area_zone_inhibited_no_32_2;*/
		}else
		{
			zone_mask =  zone_mask << (i-1);
			area_zone_include_tmp = ((alarm_service_area_t *)(area_global_ptr + area_index))->area_zone_include_32_1;
			/*area_zone_activate_tmp = ((alarm_service_area_t *)(area_global_ptr + area_index))->area_zone_activate_32_1;
			area_zone_inhibit_tmp = ((alarm_service_area_t *)(area_global_ptr + area_index))->area_zone_inhibited_no_32_1;*/
		}
		if((area_zone_include_tmp & zone_mask) && /*!<  check zone status: Included to the area or No */
			(((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->add_status == CRED_RF_DEVICES_ADDED) && /*!<  check zone status: Added or No */
			(((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_status & CRED_ZONE_STATUS_ACTIVATED ) && /*!<  check zone status: Isolated or No */
			(((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_status & CRED_ZONE_STATUS_INHIBITED_NO ) ) /*!<  check zone status: Inhibted or No */
		{
				ALARM_TRACE_INFO("Start Arming procedure of Zone %d included to Area %d status 0x%x\n",i,area_index,area_zone_include_tmp & zone_mask);
				/*!<  check zone Alarm status: Alarm, fault, sabotage, open */
				if(area_arm_mode_tmp == CRED_ARM_MODE_DISARM)
				{
					 /*!<  check for zone logic OR/AND */
					 ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_area_status &= (~area_index_tab[area_index-1]);
					if(((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_logic_type == CRED_ZONE_LOGIC_NOT_USED)
					{
						ALARM_TRACE_INFO("No logic required to disarm Zone: %d\n",i);
						if(((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_area_status == 0)
						{
							((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_arm_status = CRED_ZONE_ARM_STATUS_DISARMED;
							((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_partition_status = CRED_ARM_MODE_DISARM;
							ALARM_TRACE_INFO("Zone:%d Disarmed \n",i);
						}
					}else if(((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_logic_type == CRED_ZONE_LOGIC_OR)
					{
						ALARM_TRACE_INFO("OR logic required to disarm Zone: %d\n",i);
						if(((((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_area_status) & (((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_logic_area)) == 0)
						{
							((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_arm_status = CRED_ZONE_ARM_STATUS_DISARMED;
							((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_partition_status = CRED_ARM_MODE_DISARM;
							ALARM_TRACE_INFO("Zone:%d Disarmed \n",i);
						}
					}else
				    {
						ALARM_TRACE_INFO("And logic required to disarm Zone: %d\n",i);
						if(((((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_area_status) & (((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_logic_area))!=(((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_logic_area))
						{
							((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_arm_status = CRED_ZONE_ARM_STATUS_DISARMED;
							((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_partition_status = CRED_ARM_MODE_DISARM;
							ALARM_TRACE_INFO("Zone:%d Disarmed \n",i);
						}
					}
				}else
				{
					/*!<  check for Arm mode for the current zone p1,p2, p12 or all */
					if( (((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_partition) & zone_partition_mode )
					{
						if((((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_type == CRED_RF_ZONE_ENTRY_EXIT_1))
						 {
							 ALARM_TRACE_INFO("Exit/Entry 1 Zone: %d\n",i);
							 /* Add Sart Exit Timer 1  */
							 start_timer_entry_1_flag=1;
						  }else if((((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_type == CRED_RF_ZONE_ENTRY_EXIT_2))
						  {
							ALARM_TRACE_INFO("Exit/Entry 2 Zone: %d\n",i);
							/* Add Sart Exit Timer  2 */
							start_timer_entry_2_flag=1;
						  }else if((((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_type == CRED_RF_ZONE_ENTRY_DOOR) )
						  {
							/*Add Sart Entry Door Timer */
							ALARM_TRACE_INFO("Entry Door Zone: %d\n",i);
							start_timer_entry_door_flag=1;
						  }else
						  {
							  ALARM_TRACE_INFO("Not Exit/Entry Zone: %d\n",i);
							 
									
								  ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_area_status |=area_index_tab[area_index-1];
								  /*!<  check zone Alarm status: Alarm, fault, sabotage, open */
								  if(1/*((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_alarm_status == CRED_ZONE_ALARM_STATUS_NO_ALARM*/)
								  {
									   /*!<  check for zone logic OR/AND */
									  if(((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_logic_type == CRED_ZONE_LOGIC_NOT_USED)
									  {
										ALARM_TRACE_INFO("No logic required to arm Zone: %d\n",i);
										if(((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_forced_arm_level != CRED_FORCED_ARM_LEVEL_NOT_USED)
										{
											((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_forced_arm_level = CRED_FORCED_ARM_LEVEL_NOT_USED;
											/* TBA add event (notify ) not ready to arm *//* TBA Add check for force overwride*/
										}
										((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_arm_status = CRED_ZONE_ARM_STATUS_ARMED;
										((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_partition_status |= partition_mode;
										ALARM_TRACE_INFO("Zone:%d Armed \n",i);
									  }else if(((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_logic_type == CRED_ZONE_LOGIC_OR)
									  {
										  ALARM_TRACE_INFO("OR logic required to arm Zone: %d\n",i);
										  if(((((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_area_status) & (((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_logic_area)))
										  {
											if(((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_forced_arm_level != CRED_FORCED_ARM_LEVEL_NOT_USED)
											{
												((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_forced_arm_level = CRED_FORCED_ARM_LEVEL_NOT_USED;
												 /* TBA add event (notify ) not ready to arm *//* TBA Add check for force overwride*/
											}
											((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_arm_status = CRED_ZONE_ARM_STATUS_ARMED;
											((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_partition_status |= partition_mode;
											ALARM_TRACE_INFO("Zone:%d Armed \n",i);
										 }
									  }else
									  {
										  ALARM_TRACE_INFO("AND logic required to arm Zone: %d\n",i);
										  ALARM_TRACE_INFO("((((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_area_status: 0x%x\n",((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_area_status);
										  ALARM_TRACE_INFO("(((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_logic_area): 0x%x\n",((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_logic_area);
										  if(((((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_area_status) & (((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_logic_area))==(((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_logic_area))
										  {
											if(((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_forced_arm_level != CRED_FORCED_ARM_LEVEL_NOT_USED)
											{
												((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_forced_arm_level = CRED_FORCED_ARM_LEVEL_NOT_USED;
												 /* TBA add event (notify ) not ready to arm *//* TBA Add check for force overwride*/
											}
											((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_arm_status = CRED_ZONE_ARM_STATUS_ARMED;
											((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_partition_status |= partition_mode;
											ALARM_TRACE_INFO("Zone:%d Armed \n",i);
										  }
									  }
								  }else if(((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_forced_arm_level != CRED_FORCED_ARM_LEVEL_NOT_USED )
								  {
									  /* TBA add event (notify ) not ready to arm *//* TBA Add check for force overwride*/
									  ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_forced_arm_level = CRED_FORCED_ARM_LEVEL_NOT_USED;
									  ALARM_TRACE_INFO("Force Arm Zone: %d\n",i);
								  }else 
								  {
									  /* TBA add event (notify ) not ready to arm *//* TBA Add check for force overwride*/
									  ALARM_TRACE_INFO("Not Ready to arm Zone: %d\n",i);
								  }
							  }
					  }else
					  {
						  ALARM_TRACE_INFO("Zone: %d Partition Mode 0x%x not included\n",i,zone_partition_mode);
						  /*else if(((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_arm_status == CRED_ZONE_ARM_STATUS_ARMED)
							{
								  ALARM_TRACE_INFO("Zone: %d Already Armed update Area Status and partition mode\n",i);
								  ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_area_status |=area_index_tab[area_index-1];
								  ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_partition_status |= partition_mode;
							}*/
					  }
				}
		  }else
		  {
			  /*ALARM_TRACE_INFO("Zone: %d Include status: %d Add status:%d",i,area_zone_include_tmp & zone_mask,(((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->add_status));
			  ALARM_TRACE_INFO(" Isolated/Inhibited status: %d \n",(((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_status));*/
		  }
	}
	return CRED_MW_NO_ERROR;
}


/**
 * \fn CRED_MW_Errors_t alarm_arm_area(alarm_service_arm_mode_type_t partition_mode,uint8_t area_index)
 * \brief arm area.
 *
 * \param : alarm_service_arm_mode_type_t partition_mode,uint8_t area_index.
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_check_arm_area(uint8_t area_index,alarm_service_arm_status_t *alarm_arm_status_ptr)
{
	int i =0,time_wait;
	uint8_t delay_flag =0;
	uint8_t zone_partition_mode;
	uint8_t area_arm_mode_tmp;
	uint8_t start_timer_entry_1_flag=0;
	uint8_t start_timer_entry_2_flag=0;
	uint8_t start_timer_entry_door_flag=0;
	uint32_t zone_mask = 1;
	uint32_t area_zone_include_tmp;
	uint32_t area_zone_activate_tmp;
	uint32_t area_zone_inhibit_tmp;
	uint32_t zone_area_status_tmp; /*!<  area : (A8A7A6A5A4A3A2A1: 0/1) Armed or disarmed.*/
	alarm_service_arm_mode_type_t partition_mode;
	
	partition_mode = alarm_arm_status_ptr->alarm_arm_mode;
	area_arm_mode_tmp = ((alarm_service_area_t *)(area_global_ptr + i))->area_arm_mode_status;
	alarm_arm_status_global_ptr->alarm_arm_area = area_index;
	alarm_arm_status_global_ptr->alarm_arm_error_area = area_index;
	alarm_arm_status_global_ptr->alarm_arm_mode = partition_mode;
	alarm_arm_status_ptr->alarm_arm_area = area_index;
	alarm_arm_status_ptr->alarm_arm_error_area = area_index;
	
	ALARM_TRACE_INFO("partition_mode %d \n", partition_mode);
	switch(partition_mode)
	{
		case CRED_ARM_MODE_DISARM :
		{
			ALARM_TRACE_INFO("CRED_ARM_MODE_DISARM\n");
			area_arm_mode_tmp = CRED_ARM_MODE_DISARM;
			zone_partition_mode = 0;
		}
		break;
		case CRED_ARM_MODE_TOTAL_ARM :
		{
			ALARM_TRACE_INFO("CRED_ARM_MODE_TOTAL_ARM\n");
			area_arm_mode_tmp = CRED_ARM_MODE_TOTAL_ARM;
			delay_flag = 1;
			zone_partition_mode = CRED_ZONE_PARTITION_ALL;
		}
		break;
		case CRED_ARM_MODE_TOTAL_ARM_NO_DELAY :
		{
			ALARM_TRACE_INFO("CRED_ARM_MODE_TOTAL_ARM_NO_DELAY\n");
			area_arm_mode_tmp = CRED_ARM_MODE_TOTAL_ARM_NO_DELAY;
			delay_flag = 0;
			zone_partition_mode = CRED_ZONE_PARTITION_ALL;
		}
		break;
		case CRED_ARM_MODE_ARM_P1 :
		{
			ALARM_TRACE_INFO("CRED_ARM_MODE_ARM_P1\n");
			area_arm_mode_tmp |= CRED_ARM_MODE_ARM_P1;
			delay_flag = 1;
			zone_partition_mode = CRED_ZONE_PARTITION_1;
		}
		break;
		case CRED_ARM_MODE_ARM_P1_NO_DELAY :
		{
			ALARM_TRACE_INFO("CRED_ARM_MODE_ARM_P1_NO_DELAY\n");
			area_arm_mode_tmp |= CRED_ARM_MODE_ARM_P1_NO_DELAY;
			delay_flag = 0;
			zone_partition_mode = CRED_ZONE_PARTITION_1;
		}
		break;
		case CRED_ARM_MODE_ARM_P2 :
		{
			ALARM_TRACE_INFO("CRED_ARM_MODE_ARM_P2\n");
			area_arm_mode_tmp |= CRED_ARM_MODE_ARM_P2;
			delay_flag = 1;
			zone_partition_mode = CRED_ZONE_PARTITION_2;
		}
		break;
		case CRED_ARM_MODE_ARM_P2_NO_DELAY :
		{
			ALARM_TRACE_INFO("CRED_ARM_MODE_ARM_P2_NO_DELAY\n");
			area_arm_mode_tmp |= CRED_ARM_MODE_ARM_P2_NO_DELAY;
			delay_flag = 0;
			zone_partition_mode = CRED_ZONE_PARTITION_2;
		}
		break;
		case CRED_ARM_MODE_ARM_P12 :
		{
			ALARM_TRACE_INFO("CRED_ARM_MODE_ARM_P12\n");
			area_arm_mode_tmp |= CRED_ARM_MODE_ARM_P12;
			delay_flag = 1;
			zone_partition_mode = CRED_ZONE_PARTITION_1_2;
		}
		break;
		case CRED_ARM_MODE_ARM_P12_NO_DELAY :
		{
			ALARM_TRACE_INFO("CRED_ARM_MODE_ARM_P12_NO_DELAY\n");
			area_arm_mode_tmp |= CRED_ARM_MODE_ARM_P12_NO_DELAY;
			delay_flag = 0;
			zone_partition_mode = CRED_ZONE_PARTITION_1_2;
		}
		break;
		default:
			ALARM_TRACE_ERROR("Not supported Arm mode : %d \n",partition_mode);
		break;
	}

	/*!<  Reset forced arm option */
	((alarm_service_area_t *)(area_global_ptr + area_index))->area_forced_arm_level = CRED_FORCED_ARM_LEVEL_NOT_USED;
	/*!<  Setting of Area Arm mode */
	//((alarm_service_area_t *)(area_global_ptr + area_index))->area_arm_mode_status = area_arm_mode_tmp;
	/*!<  Start Arm/Disarm Zone */
	for(i=1;i<=MAX_RF_ZONE_NUMBER;i++)
	{
		/*!<  check zone status: Included to the area or No */
		zone_mask = 1;
		if( i > 32 )
		{
			zone_mask =  zone_mask << (i-33);
			area_zone_include_tmp = ((alarm_service_area_t *)(area_global_ptr + area_index))->area_zone_include_32_2;
		}else
		{
			zone_mask =  zone_mask << (i-1);
			area_zone_include_tmp = ((alarm_service_area_t *)(area_global_ptr + area_index))->area_zone_include_32_1;
		}
		if((area_zone_include_tmp & zone_mask) && /*!<  check zone status: Included to the area or No */
			(((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->add_status == CRED_RF_DEVICES_ADDED) && /*!<  check zone status: Added or No */
			(((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_status & CRED_ZONE_STATUS_ACTIVATED ) && /*!<  check zone status: Isolated or No */
			(((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_status & CRED_ZONE_STATUS_INHIBITED_NO ) ) /*!<  check zone status: Inhibted or No */
		{
			ALARM_TRACE_INFO("Start Arming procedure of Zone %d included to Area %d status 0x%x\n",i,area_index,area_zone_include_tmp & zone_mask);
			/*!<  read zone status: Armed or No */
			alarm_arm_status_global_ptr->alarm_arm_error_zone = i;
			alarm_arm_status_ptr->alarm_arm_error_zone = i;
			zone_area_status_tmp = ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_area_status;
			/*!<  check for Arm mode for the current zone p1,p2, p12 or all */
			if( (((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_partition) & zone_partition_mode )
			{
				if((((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_type == CRED_RF_ZONE_ENTRY_EXIT_1))
				{
					ALARM_TRACE_INFO("Exit/Entry 1 Zone: %d\n",i);
					/* Add Sart Exit Timer 1  */
					//start_timer_entry_1_flag=1;
				}else if((((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_type == CRED_RF_ZONE_ENTRY_EXIT_2))
				{
					ALARM_TRACE_INFO("Exit/Entry 2 Zone: %d\n",i);
					/* Add Sart Exit Timer  2 */
					//start_timer_entry_2_flag=1;
				}else if((((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_type == CRED_RF_ZONE_ENTRY_DOOR) )
				{
					/*Add Sart Entry Door Timer */
					ALARM_TRACE_INFO("Entry Door Zone: %d\n",i);
					//start_timer_entry_door_flag=1;
				}else
				{
						ALARM_TRACE_INFO("Not Exit/Entry Zone: %d\n",i);	 
						zone_area_status_tmp |=area_index_tab[area_index-1];
						/*!<  check zone Alarm status: Alarm, fault, sabotage, open */
						if(((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_alarm_status == CRED_ZONE_ALARM_STATUS_NO_ALARM)
						{
							ALARM_TRACE_INFO("Zone:%d Ready for Arming \n",i);
							((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_forced_arm_level = CRED_FORCED_ARM_LEVEL_NOT_USED;
							alarm_arm_status_global_ptr->alarm_arm_status = CRED_ARM_PROCEDURE_READY;
							alarm_arm_status_ptr->alarm_arm_status = CRED_ARM_PROCEDURE_READY;
						}else
						{
								alarm_arm_status_global_ptr->alarm_arm_error = CRED_ZONE_ALARM_STATUS_NO_ALARM;
								alarm_arm_status_ptr->alarm_arm_error = CRED_ZONE_ALARM_STATUS_NO_ALARM;
								if((((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_alarm_status & CRED_ZONE_ALARM_STATUS_ALARM) == CRED_ZONE_ALARM_STATUS_ALARM)
								{
									  /* TBA add event (notify ) not ready to arm *//* TBA Add check for force override*/
									 ALARM_TRACE_INFO("Not Ready to arm Zone %d : Alarm\n",i);
									 alarm_arm_status_global_ptr->alarm_arm_error |= CRED_ZONE_ALARM_STATUS_ALARM;
									 alarm_arm_status_ptr->alarm_arm_error |= CRED_ZONE_ALARM_STATUS_ALARM;
									 ALARM_TRACE_INFO("Check option %d : Alarm\n",alarm_arm_status_global_ptr->alarm_arm_check_force);
									 if(alarm_arm_status_global_ptr->alarm_arm_check_force)
									 {
										  alarm_arm_status_global_ptr->alarm_arm_status = CRED_ARM_PROCEDURE_WAIT_FORCE_ARM;
										  time_wait = MAX_WAIT_ANSWER_TIME;
										  ALARM_TRACE_INFO("Check for forced arm Zone %d : Alarm\n",i);
										  while((alarm_arm_status_global_ptr->alarm_arm_status == CRED_ARM_PROCEDURE_WAIT_FORCE_ARM) && time_wait)
										  {
											  time_wait--;
											  usleep(10);
										  }
										  if(alarm_arm_status_ptr->alarm_arm_status == CRED_ARM_PROCEDURE_FORCE_ARM_YES)
										  {
											 /*TBA check for user level (required l2 or l3 )*/
											((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_forced_arm_level = CRED_FORCED_ARM_LEVEL2;
											ALARM_TRACE_INFO("Enabled forced arm Zone %d : Alarm\n",i);
										  }else
										  {
											((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_forced_arm_level = CRED_FORCED_ARM_LEVEL_NOT_USED;
										  }
									  }else
									  {
										  ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_forced_arm_level = CRED_FORCED_ARM_LEVEL_NOT_USED;
									  }
									  /* check for forced option */
									  if(((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_forced_arm_level == CRED_FORCED_ARM_LEVEL_NOT_USED)
									  {
										  ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_forced_arm_level = CRED_FORCED_ARM_LEVEL_NOT_USED;
										  alarm_arm_status_global_ptr->alarm_arm_status = CRED_ARM_PROCEDURE_BLOCKED;
										  alarm_arm_status_ptr->alarm_arm_status = CRED_ARM_PROCEDURE_BLOCKED;
										  ALARM_TRACE_INFO("Arming Zone %d blocked: Alarm\n",i);
										  /* TBA add event (notify ) */
										  return CRED_MW_NO_ERROR;
									  }
								}
								if((((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_alarm_status & CRED_ZONE_ALARM_STATUS_FAULT) == CRED_ZONE_ALARM_STATUS_FAULT)
								{
									 /* TBA add event (notify ) not ready to arm *//* TBA Add check for force override*/
									 ALARM_TRACE_INFO("Not Ready to arm Zone %d : Fault\n",i);
									 alarm_arm_status_global_ptr->alarm_arm_error |= CRED_ZONE_ALARM_STATUS_FAULT;
									 alarm_arm_status_ptr->alarm_arm_error |= CRED_ZONE_ALARM_STATUS_FAULT;
									 ALARM_TRACE_INFO("Check option %d : Fault\n",alarm_arm_status_global_ptr->alarm_arm_check_force);
									 if(alarm_arm_status_global_ptr->alarm_arm_check_force)
									 {
										  alarm_arm_status_global_ptr->alarm_arm_status = CRED_ARM_PROCEDURE_WAIT_FORCE_ARM;
										  time_wait = MAX_WAIT_ANSWER_TIME;
										  ALARM_TRACE_INFO("Check for forced arm Zone %d : Fault\n",i);
										  while((alarm_arm_status_global_ptr->alarm_arm_status == CRED_ARM_PROCEDURE_WAIT_FORCE_ARM) && time_wait)
										  {
											  time_wait--;
											  usleep(10);
										  }
										  if(alarm_arm_status_ptr->alarm_arm_status == CRED_ARM_PROCEDURE_FORCE_ARM_YES)
										  {
											 /*TBA check for user level (required l2 or l3 )*/
											((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_forced_arm_level = CRED_FORCED_ARM_LEVEL2;
											ALARM_TRACE_INFO("Enabled forced arm Zone %d : Fault\n",i);
										  }else
										  {
											((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_forced_arm_level = CRED_FORCED_ARM_LEVEL_NOT_USED;
										  }
									  }else
									  {
										  ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_forced_arm_level = CRED_FORCED_ARM_LEVEL_NOT_USED;
									  }
									  /* check for forced option */
									  if(((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_forced_arm_level == CRED_FORCED_ARM_LEVEL_NOT_USED)
									  {
										  ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_forced_arm_level = CRED_FORCED_ARM_LEVEL_NOT_USED;
										  alarm_arm_status_global_ptr->alarm_arm_status = CRED_ARM_PROCEDURE_BLOCKED;
										  alarm_arm_status_ptr->alarm_arm_status = CRED_ARM_PROCEDURE_BLOCKED;
										  ALARM_TRACE_INFO("Arming Zone %d blocked: Fault\n",i);
										  /* TBA add event (notify ) */
										  return CRED_MW_NO_ERROR;
									  }
								}
								if((((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_alarm_status & CRED_ZONE_ALARM_STATUS_SABOTAGE) == CRED_ZONE_ALARM_STATUS_SABOTAGE)
								{
									  /* TBA add event (notify ) not ready to arm *//* TBA Add check for force override*/
									  ALARM_TRACE_INFO("Not Ready to arm Zone %d : Sabotage\n",i);
									  alarm_arm_status_global_ptr->alarm_arm_error |= CRED_ZONE_ALARM_STATUS_SABOTAGE;
									  alarm_arm_status_ptr->alarm_arm_error |= CRED_ZONE_ALARM_STATUS_SABOTAGE;
									  ALARM_TRACE_INFO("Check option %d : Sabotage\n",alarm_arm_status_global_ptr->alarm_arm_check_force);
									  if(alarm_arm_status_global_ptr->alarm_arm_check_force)
									  {
										  alarm_arm_status_global_ptr->alarm_arm_status = CRED_ARM_PROCEDURE_WAIT_FORCE_ARM;
										  time_wait = MAX_WAIT_ANSWER_TIME;
										  ALARM_TRACE_INFO("Check for forced arm Zone %d : Sabotage\n",i);
										  while((alarm_arm_status_global_ptr->alarm_arm_status == CRED_ARM_PROCEDURE_WAIT_FORCE_ARM) && time_wait)
										  {
											  time_wait--;
											  usleep(10);
											   /* test only*/
											  alarm_arm_status_ptr->alarm_arm_status = CRED_ARM_PROCEDURE_FORCE_ARM_YES;
										  }
										  if(alarm_arm_status_ptr->alarm_arm_status == CRED_ARM_PROCEDURE_FORCE_ARM_YES)
										  {
											/*TBA check for user level (required l3 )*/
											((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_forced_arm_level = CRED_FORCED_ARM_LEVEL3;
											((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_forced_arm_level = CRED_FORCED_ARM_LEVEL_NOT_USED;
											ALARM_TRACE_INFO("Enabled forced arm Zone %d : Sabotage\n",i);
										  }else
										  {
											((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_forced_arm_level = CRED_FORCED_ARM_LEVEL_NOT_USED;
										  }
									  }else
									  {
										  ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_forced_arm_level = CRED_FORCED_ARM_LEVEL_NOT_USED;
									  }
									  /* check for forced option */
									  if(((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_forced_arm_level == CRED_FORCED_ARM_LEVEL_NOT_USED)
									  {
										  ((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_forced_arm_level = CRED_FORCED_ARM_LEVEL_NOT_USED;
										  alarm_arm_status_global_ptr->alarm_arm_status = CRED_ARM_PROCEDURE_BLOCKED;
										  alarm_arm_status_ptr->alarm_arm_status = CRED_ARM_PROCEDURE_BLOCKED;
										  ALARM_TRACE_INFO("Arming Zone %d blocked: Sabotage\n",i);
										  /* TBA add event (notify ) */
										  return CRED_MW_NO_ERROR;
									  }
								}
						}
					}/*else
					{
						ALARM_TRACE_INFO("Zone: %d Include status: %d Add status:%d",i,area_zone_include_tmp & zone_mask,(((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->add_status));
						ALARM_TRACE_INFO(" Isolated/Inhibited status: %d \n",(((alarm_service_rf_zone_t *)(rf_zone_global_ptr + i))->zone_status));
					}*/
			}
			/*else
			{
				ALARM_TRACE_INFO("Zone: %d Partition Mode 0x%x not included\n",i,zone_partition_mode);
			}*/
		  }
		
	}
	alarm_arm_status_global_ptr->alarm_arm_status = CRED_ARM_PROCEDURE_READY;
	alarm_arm_status_ptr->alarm_arm_status = CRED_ARM_PROCEDURE_READY;
	ALARM_TRACE_INFO("Area:%d Ready for Arming \n",area_index);
	return CRED_MW_NO_ERROR;
}
/**
 * \fn CRED_MW_Errors_t alarm_start_arm_procedure(alarm_service_arm_status_t *alarm_arm_status_ptr)
 * \brief arm area.
 *
 * \param : alarm_service_arm_status_t *alarm_arm_status_ptr.
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_start_arm_procedure(alarm_service_arm_status_t *alarm_arm_status_ptr)
{
	arm_event_to_alarm_service.As_event = CRED_ALARM_ARM_PROCEDURE;
	arm_event_to_alarm_service.event_detail = (uint8_t *)alarm_arm_status_ptr;
	alarm_service_notify(arm_event_to_alarm_service);
	return CRED_MW_NO_ERROR;
}
/**
 * \fn CRED_MW_Errors_t alarm_start_arm_total_procedure(uint8_t area_index, uint32_t *alarm_arm_status_ptr,uint8_t arm_check_force_flag)
 * \brief total arm area/system.
 *
 * \param : uint8_t area_index, uint32_t *alarm_arm_status_ptr
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_start_arm_total_procedure(uint8_t area_index, uint32_t *alarm_arm_status_ptr,uint8_t arm_check_force_flag)
{
	
	memset(alarm_arm_status_global_ptr,0,sizeof(alarm_service_arm_status_t));
	alarm_arm_status_global_ptr->alarm_arm_status = CRED_ARM_PROCEDURE_START;
	
	if(area_index > MAX_AREA_NUMBER)
	{
		ALARM_TRACE_ERROR("Area index %d Not supported\n",area_index);
		*alarm_arm_status_ptr = NULL;
		return CRED_MW_NO_ERROR;
	}
	alarm_arm_status_global_ptr->alarm_arm_area = area_index;
	alarm_arm_status_global_ptr->alarm_arm_mode = CRED_ARM_MODE_TOTAL_ARM;
	alarm_arm_status_global_ptr->alarm_arm_check_force = arm_check_force_flag;
	ALARM_TRACE_INFO("Check option %d : arm_check_force_flag %d\n",alarm_arm_status_global_ptr->alarm_arm_check_force,arm_check_force_flag);
	arm_event_to_alarm_service.As_event = CRED_ALARM_ARM_PROCEDURE;
	arm_event_to_alarm_service.event_detail = (uint8_t *)alarm_arm_status_global_ptr; /*TBV for back-up*/
	alarm_service_notify(arm_event_to_alarm_service);
	
	*alarm_arm_status_ptr = alarm_arm_status_global_ptr;
	return CRED_MW_NO_ERROR;
}
/**
 * \fn CRED_MW_Errors_t alarm_start_arm_total_procedure(uint8_t area_index, uint32_t *alarm_arm_status_ptr,uint8_t arm_check_force_flag)
 * \brief total arm no dealy area/system.
 *
 * \param : uint8_t area_index, uint32_t *alarm_arm_status_ptr
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_start_arm_total_nodelay_procedure(uint8_t area_index, uint32_t *alarm_arm_status_ptr,uint8_t arm_check_force_flag)
{
	
	memset(alarm_arm_status_global_ptr,0,sizeof(alarm_service_arm_status_t));
	alarm_arm_status_global_ptr->alarm_arm_status = CRED_ARM_PROCEDURE_START;
	
	if(area_index > MAX_AREA_NUMBER)
	{
		ALARM_TRACE_ERROR("Area index %d Not supported\n",area_index);
		*alarm_arm_status_ptr = NULL;
		return CRED_MW_NO_ERROR;
	}
	alarm_arm_status_global_ptr->alarm_arm_area = area_index;
	alarm_arm_status_global_ptr->alarm_arm_mode = CRED_ARM_MODE_TOTAL_ARM_NO_DELAY;
	alarm_arm_status_global_ptr->alarm_arm_check_force = arm_check_force_flag;
	arm_event_to_alarm_service.As_event = CRED_ALARM_ARM_PROCEDURE;
	arm_event_to_alarm_service.event_detail = (uint8_t *)alarm_arm_status_global_ptr; /*TBV for back-up*/
	alarm_service_notify(arm_event_to_alarm_service);
	
	*alarm_arm_status_ptr = alarm_arm_status_global_ptr;
	return CRED_MW_NO_ERROR;
}
/**
 * \fn CRED_MW_Errors_t alarm_start_arm_p1_procedure(uint8_t area_index, uint32_t *alarm_arm_status_ptr,uint8_t arm_check_force_flag)
 * \brief p1 arm area/system.
 *
 * \param : uint8_t area_index, uint32_t *alarm_arm_status_ptr
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_start_arm_p1_procedure(uint8_t area_index, uint32_t *alarm_arm_status_ptr,uint8_t arm_check_force_flag)
{
	
	memset(alarm_arm_status_global_ptr,0,sizeof(alarm_service_arm_status_t));
	alarm_arm_status_global_ptr->alarm_arm_status = CRED_ARM_PROCEDURE_START;
	
	if(area_index > MAX_AREA_NUMBER)
	{
		ALARM_TRACE_ERROR("Area index %d Not supported\n",area_index);
		*alarm_arm_status_ptr = NULL;
		return CRED_MW_NO_ERROR;
	}
	alarm_arm_status_global_ptr->alarm_arm_area = area_index;
	alarm_arm_status_global_ptr->alarm_arm_mode = CRED_ARM_MODE_ARM_P1;
	alarm_arm_status_global_ptr->alarm_arm_check_force = arm_check_force_flag;
	arm_event_to_alarm_service.As_event = CRED_ALARM_ARM_PROCEDURE;
	arm_event_to_alarm_service.event_detail = (uint8_t *)alarm_arm_status_global_ptr; /*TBV for back-up*/
	alarm_service_notify(arm_event_to_alarm_service);
	
	*alarm_arm_status_ptr = alarm_arm_status_global_ptr;
	return CRED_MW_NO_ERROR;
}
/**
 * \fn CRED_MW_Errors_t alarm_start_arm_p1_nodelay_procedure(uint8_t area_index, uint32_t *alarm_arm_status_ptr,uint8_t arm_check_force_flag)
 * \brief p1 arm no dealy area/system.
 *
 * \param : uint8_t area_index, uint32_t *alarm_arm_status_ptr
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_start_arm_p1_nodelay_procedure(uint8_t area_index, uint32_t *alarm_arm_status_ptr,uint8_t arm_check_force_flag)
{
	
	memset(alarm_arm_status_global_ptr,0,sizeof(alarm_service_arm_status_t));
	alarm_arm_status_global_ptr->alarm_arm_status = CRED_ARM_PROCEDURE_START;
	
	if(area_index > MAX_AREA_NUMBER)
	{
		ALARM_TRACE_ERROR("Area index %d Not supported\n",area_index);
		*alarm_arm_status_ptr = NULL;
		return CRED_MW_NO_ERROR;
	}
	alarm_arm_status_global_ptr->alarm_arm_area = area_index;
	alarm_arm_status_global_ptr->alarm_arm_mode = CRED_ARM_MODE_ARM_P1_NO_DELAY;
	alarm_arm_status_global_ptr->alarm_arm_check_force = arm_check_force_flag;
	arm_event_to_alarm_service.As_event = CRED_ALARM_ARM_PROCEDURE;
	arm_event_to_alarm_service.event_detail = (uint8_t *)alarm_arm_status_global_ptr; /*TBV for back-up*/
	alarm_service_notify(arm_event_to_alarm_service);
	
	*alarm_arm_status_ptr = alarm_arm_status_global_ptr;
	return CRED_MW_NO_ERROR;
}
/**
 * \fn CRED_MW_Errors_t alarm_start_arm_p2_procedure(uint8_t area_index, uint32_t *alarm_arm_status_ptr,uint8_t arm_check_force_flag)
 * \brief p2 arm area/system.
 *
 * \param : uint8_t area_index, uint32_t *alarm_arm_status_ptr
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_start_arm_p2_procedure(uint8_t area_index, uint32_t *alarm_arm_status_ptr,uint8_t arm_check_force_flag)
{
	
	memset(alarm_arm_status_global_ptr,0,sizeof(alarm_service_arm_status_t));
	alarm_arm_status_global_ptr->alarm_arm_status = CRED_ARM_PROCEDURE_START;
	
	if(area_index > MAX_AREA_NUMBER)
	{
		ALARM_TRACE_ERROR("Area index %d Not supported\n",area_index);
		*alarm_arm_status_ptr = NULL;
		return CRED_MW_NO_ERROR;
	}
	alarm_arm_status_global_ptr->alarm_arm_area = area_index;
	alarm_arm_status_global_ptr->alarm_arm_mode = CRED_ARM_MODE_ARM_P2;
	alarm_arm_status_global_ptr->alarm_arm_check_force = arm_check_force_flag;
	arm_event_to_alarm_service.As_event = CRED_ALARM_ARM_PROCEDURE;
	arm_event_to_alarm_service.event_detail = (uint8_t *)alarm_arm_status_global_ptr; /*TBV for back-up*/
	alarm_service_notify(arm_event_to_alarm_service);
	
	*alarm_arm_status_ptr = alarm_arm_status_global_ptr;
	return CRED_MW_NO_ERROR;
}
/**
 * \fn CRED_MW_Errors_t alarm_start_arm_p2_nodelay_procedure(uint8_t area_index, uint32_t *alarm_arm_status_ptr,uint8_t arm_check_force_flag)
 * \brief p2 arm no dealy area/system.
 *
 * \param : uint8_t area_index, uint32_t *alarm_arm_status_ptr
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_start_arm_p2_nodelay_procedure(uint8_t area_index, uint32_t *alarm_arm_status_ptr,uint8_t arm_check_force_flag)
{
	
	memset(alarm_arm_status_global_ptr,0,sizeof(alarm_service_arm_status_t));
	alarm_arm_status_global_ptr->alarm_arm_status = CRED_ARM_PROCEDURE_START;
	
	if(area_index > MAX_AREA_NUMBER)
	{
		ALARM_TRACE_ERROR("Area index %d Not supported\n",area_index);
		*alarm_arm_status_ptr = NULL;
		return CRED_MW_NO_ERROR;
	}
	alarm_arm_status_global_ptr->alarm_arm_area = area_index;
	alarm_arm_status_global_ptr->alarm_arm_mode = CRED_ARM_MODE_ARM_P2_NO_DELAY;
	alarm_arm_status_global_ptr->alarm_arm_check_force = arm_check_force_flag;
	arm_event_to_alarm_service.As_event = CRED_ALARM_ARM_PROCEDURE;
	arm_event_to_alarm_service.event_detail = (uint8_t *)alarm_arm_status_global_ptr; /*TBV for back-up*/
	alarm_service_notify(arm_event_to_alarm_service);
	
	*alarm_arm_status_ptr = alarm_arm_status_global_ptr;
	return CRED_MW_NO_ERROR;
}
/**
 * \fn CRED_MW_Errors_t alarm_start_arm_p12_procedure(uint8_t area_index, uint32_t *alarm_arm_status_ptr,uint8_t arm_check_force_flag)
 * \brief p12 arm area/system.
 *
 * \param : uint8_t area_index, uint32_t *alarm_arm_status_ptr
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_start_arm_p12_procedure(uint8_t area_index, uint32_t *alarm_arm_status_ptr,uint8_t arm_check_force_flag)
{
	
	memset(alarm_arm_status_global_ptr,0,sizeof(alarm_service_arm_status_t));
	alarm_arm_status_global_ptr->alarm_arm_status = CRED_ARM_PROCEDURE_START;
	
	if(area_index > MAX_AREA_NUMBER)
	{
		ALARM_TRACE_ERROR("Area index %d Not supported\n",area_index);
		*alarm_arm_status_ptr = NULL;
		return CRED_MW_NO_ERROR;
	}
	alarm_arm_status_global_ptr->alarm_arm_area = area_index;
	alarm_arm_status_global_ptr->alarm_arm_mode = CRED_ARM_MODE_ARM_P12;
	alarm_arm_status_global_ptr->alarm_arm_check_force = arm_check_force_flag;
	arm_event_to_alarm_service.As_event = CRED_ALARM_ARM_PROCEDURE;
	arm_event_to_alarm_service.event_detail = (uint8_t *)alarm_arm_status_global_ptr; /*TBV for back-up*/
	alarm_service_notify(arm_event_to_alarm_service);
	
	*alarm_arm_status_ptr = alarm_arm_status_global_ptr;
	return CRED_MW_NO_ERROR;
}
/**
 * \fn CRED_MW_Errors_t alarm_start_arm_p12_nodelay_procedure(uint8_t area_index, uint32_t *alarm_arm_status_ptr,uint8_t arm_check_force_flag)
 * \brief p12 arm no dealy area/system.
 *
 * \param : uint8_t area_index, uint32_t *alarm_arm_status_ptr
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_start_arm_p12_nodelay_procedure(uint8_t area_index, uint32_t *alarm_arm_status_ptr,uint8_t arm_check_force_flag)
{
	
	memset(alarm_arm_status_global_ptr,0,sizeof(alarm_service_arm_status_t));
	alarm_arm_status_global_ptr->alarm_arm_status = CRED_ARM_PROCEDURE_START;
	
	if(area_index > MAX_AREA_NUMBER)
	{
		ALARM_TRACE_ERROR("Area index %d Not supported\n",area_index);
		*alarm_arm_status_ptr = NULL;
		return CRED_MW_NO_ERROR;
	}
	alarm_arm_status_global_ptr->alarm_arm_area = area_index;
	alarm_arm_status_global_ptr->alarm_arm_mode = CRED_ARM_MODE_ARM_P12_NO_DELAY;
	alarm_arm_status_global_ptr->alarm_arm_check_force = arm_check_force_flag;
	arm_event_to_alarm_service.As_event = CRED_ALARM_ARM_PROCEDURE;
	arm_event_to_alarm_service.event_detail = (uint8_t *)alarm_arm_status_global_ptr; /*TBV for back-up*/
	alarm_service_notify(arm_event_to_alarm_service);
	
	*alarm_arm_status_ptr = alarm_arm_status_global_ptr;
	return CRED_MW_NO_ERROR;
}
/**
 * \fn CRED_MW_Errors_t alarm_arm_procedure(alarm_service_arm_status_t *alarm_arm_status_ptr)
 * \brief arm area.
 *
 * \param : alarm_service_arm_status_t *alarm_arm_status_ptr.
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_arm_procedure(alarm_service_arm_status_t *alarm_arm_status_ptr)
{
	
	/*!<  system arm */
	int i =0,area_min,area_max;
	
	/*!<  check system status */
	if(alarm_arm_status_ptr->alarm_arm_area == 0)
	{
		area_min = 1;
		area_max = MAX_AREA_NUMBER;
		ALARM_TRACE_INFO("Arming procedure System: area_min %d area_max %d\n",area_min,area_max);
	}else
	{
		area_min = alarm_arm_status_ptr->alarm_arm_area;
		area_max = alarm_arm_status_ptr->alarm_arm_area;
		ALARM_TRACE_INFO("Arming procedure Area area_min  %d area_max %d\n",area_min,area_max);
	}
	
	/*!<  Start Check Area */
	for(i=area_min;i<=area_max;i++)
	{
		ALARM_TRACE_INFO("Start check Area %d \n",i);
		alarm_check_arm_area(i,alarm_arm_status_ptr);
		if(alarm_arm_status_global_ptr->alarm_arm_status == CRED_ARM_PROCEDURE_BLOCKED)
		{
			ALARM_TRACE_INFO("Arming procedure blocked Area %d procedure \n",i);
			i=area_max+1;
		}
	}
	/*!<  Start Arm/Disarm Area */
	if(alarm_arm_status_global_ptr->alarm_arm_status == CRED_ARM_PROCEDURE_READY)
	{
		for(i=area_min;i<=area_max;i++)
		{
			ALARM_TRACE_INFO("Start Arming Area %d procedure to Arm mode \n",i);
			alarm_arm_area(alarm_arm_status_ptr->alarm_arm_mode,i);
		}
		ALARM_TRACE_INFO("Arming procedure done\n");
	}
	
	return CRED_MW_NO_ERROR;
}
/**
 * \fn CRED_MW_Errors_t arm_all_system(void)
 * \brief arm all system.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t arm_all_system(void)
{
	alarm_arm_system(CRED_ARM_MODE_TOTAL_ARM);
	ALARM_TRACE_INFO("arm mode: %d  \n",arm_mode_global);
	return CRED_MW_NO_ERROR;
}

/**
 * \fn CRED_MW_Errors_t disarm_all_system(void)
 * \brief arm all system.
 *
 * \param No parameters.
 * \return No parameter.
 */
CRED_MW_Errors_t disarm_all_system(void)
{
	alarm_arm_system(CRED_ARM_MODE_DISARM);
	ALARM_TRACE_INFO("arm mode: %d  \n",arm_mode_global);
	return CRED_MW_NO_ERROR;
}

CRED_MW_Errors_t start_camera(int fb )
{

	Camera_SetConfigs(fb);

	return CRED_MW_NO_ERROR;
}

CRED_MW_Errors_t stop_camera(int fb )
{

	 Camera_shutdown(fb);

	return CRED_MW_NO_ERROR;
}

/**
 * \fn CRED_MW_Errors_t alarm_get_network_info(uint32_t *alarm_network_info_ptr)
 * \brief get alarm system network info.
 *
 * \param : uint32_t *alarm_network_info_ptr
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_get_system_info(uint32_t *alarm_system_info_ptr)
{
	memcpy(alarm_system_info_ptr,alarm_system_info_global_ptr,sizeof(alarm_system_info_t));

	return CRED_MW_NO_ERROR;
}
/**
 * \fn CRED_MW_Errors_t alarm_load_system_info(void)
 * \brief get alarm system network info.
 *
 * \param : uint32_t *alarm_network_info_ptr
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_load_system_info(void)
{
	FILE *fp_sys_info=NULL;
	/*Open installer Code file:*/
	fp_sys_info = fopen(CRED_SYS_INF_FILE_PATH, "r");
	if(fp_sys_info == NULL)
	{
		/*File Does Not Exit => Create the File*/
		fp_sys_info = fopen(CRED_SYS_INF_FILE_PATH, "w+");

			alarm_system_info_global_ptr->alarm_network.alarm_ethernet_status=CRED_ETHERNET_NOK;	/*!< Ethernet Status : Connected/Not Connected*/
			alarm_system_info_global_ptr->alarm_network.alarm_ethernet_dhcp_status =  CRED_DISABLE;	/*!<  DHCP Status: ENABLED/DISABLED*/

			memset(alarm_system_info_global_ptr->alarm_network.alarm_network_mac_address,0,MAX_MAC_ADDRESS_STR);/*!< Mac address */
			sprintf(alarm_system_info_global_ptr->alarm_network.alarm_network_mac_address,"00:25:29:11:22:33",0);
			memset(alarm_system_info_global_ptr->alarm_network.alarm_ethernet_ip_address,0,MAX_IP_ADDRESS_STR); /*!< Ethernet IP address */
			sprintf(alarm_system_info_global_ptr->alarm_network.alarm_ethernet_ip_address,"172.22.0.140",0);
			memset(alarm_system_info_global_ptr->alarm_network.alarm_ethernet_ip_mask,0,MAX_IP_ADDRESS_STR); /*!< Ethernet IP Mask */
			sprintf(alarm_system_info_global_ptr->alarm_network.alarm_ethernet_ip_mask,"255.255.0.0",0);
			memset(alarm_system_info_global_ptr->alarm_network.alarm_ethernet_ip_gateway,0,MAX_IP_ADDRESS_STR); /*!< Ethernet IP address */
			sprintf(alarm_system_info_global_ptr->alarm_network.alarm_ethernet_ip_mask,"172.22.0.1",0);
			memset(alarm_system_info_global_ptr->alarm_network.alarm_ethernet_ip_dns1,0,MAX_IP_ADDRESS_STR); /*!< Network DNS 1 address */
			memset(alarm_system_info_global_ptr->alarm_network.alarm_ethernet_ip_dns2,0,MAX_IP_ADDRESS_STR); /*!< Network DNS 2 address */
			/*!< Wifi interface */
			alarm_system_info_global_ptr->alarm_network.alarm_wifi_status = CRED_WIFI_AP_ENABLED | CRED_WIFI_CLIENT_ENABLED;	/*!<  Wifi Status: OFF/AP/CLIENT*/
			alarm_system_info_global_ptr->alarm_network.alarm_wifi_client_signal_status = CRED_WIFI_CLIENT_NOT_USED;	/*!<  Wifi Signal level */
			memset(alarm_system_info_global_ptr->alarm_network.alarm_wifi_client_network_name,0,CRED_WIFI_AP_NAME_STR);	/*!< wifi description. */
			memset(alarm_system_info_global_ptr->alarm_network.alarm_wifi_client_network_key,0,CRED_WIFI_AP_KEY_STR);	/*!< wifi description. */
			memset(alarm_system_info_global_ptr->alarm_network.alarm_wifi_ip_address,0,MAX_IP_ADDRESS_STR); /*!< Wifi IP address */
			memset(alarm_system_info_global_ptr->alarm_network.alarm_wifi_ip_mask,0,MAX_IP_ADDRESS_STR); /*!< WiFi IP Mask */
			memset(alarm_system_info_global_ptr->alarm_network.alarm_wifi_ip_gateway,0,MAX_IP_ADDRESS_STR); /*!< WiFi IP address */
			memset(alarm_system_info_global_ptr->alarm_network.alarm_wifi_ip_dns1,0,MAX_IP_ADDRESS_STR); /*!< WiFi DNS 1 address */
			memset(alarm_system_info_global_ptr->alarm_network.alarm_wifi_ip_dns2,0,MAX_IP_ADDRESS_STR); /*!< WiFi DNS 2 address */
			memset(alarm_system_info_global_ptr->alarm_network.alarm_wifi_ap_name,0,CRED_WIFI_AP_NAME_STR);	/*!< WiFi AP description. */
			sprintf(alarm_system_info_global_ptr->alarm_network.alarm_wifi_ap_name,"Comelit_AP",0);
			memset(alarm_system_info_global_ptr->alarm_network.alarm_wifi_ap_key,0,CRED_WIFI_AP_KEY_STR);/*!< WiFi WiFi description. */
			sprintf(alarm_system_info_global_ptr->alarm_network.alarm_wifi_ap_key,"123456789",0);
			/*!< Power source status */
			alarm_system_info_global_ptr->alarm_power.alarm_power_source_status = CRED_ON;	/*!< Power Status : ON/OFF*/
			alarm_system_info_global_ptr->alarm_power.alarm_battery_status=CRED_BATTERY_NO_BATTERY;/*!<  bettery Status: Ok/fault*/
			/*!< 3g Module status */
			alarm_system_info_global_ptr->alarm_3g.alarm_3g_status=CRED_OFF;	/*!< 3G module Status : ON/OFF*/
			alarm_system_info_global_ptr->alarm_3g.alarm_3g_sim_signal_status = CRED_3G_SIM_CARD_NO_SIM;/*!<  SIM Status: Ok/NOK*/
			alarm_system_info_global_ptr->alarm_3g.alarm_3g_data_network_status = CRED_3G_NETWORK_NO_NETWORK;/*!<  3G Data network Status: Ok/NOK*/
			memset(alarm_system_info_global_ptr->alarm_3g.alarm_3g_sim_card_number,0,MAX_PHONE_NUMBER_STR);/*!<  SIM CARD phone number*/
			memset(alarm_system_info_global_ptr->alarm_3g.alarm_3g_sim_card_serial,0,MAX_PHONE_NUMBER_STR);/*!<  SIM CARD phone serial*/
			memset(alarm_system_info_global_ptr->alarm_3g.alarm_3g_sim_card_pin_code,0,MAX_PIN_CODE_NUMBER);/*!<  SIM CARD pin code*/
			/*!< 3g Module MMS setting */
			alarm_system_info_global_ptr->alarm_3g.alarm_3g_mms_character_type=CRED_3G_MMS_CHARACTER_TYPE_ASCII;/*!<  MMS character type*/
			alarm_system_info_global_ptr->alarm_3g.alarm_3g_mms_character_file_type=CRED_3G_MMS_CHARACTER_TYPE_ASCII;/*!<  MMS character file type*/
			memset(alarm_system_info_global_ptr->alarm_3g.alarm_3g_mms_ap,0,CRED_3G_MMS_AP_STR);/*!<  MMS access point*/
			memset(alarm_system_info_global_ptr->alarm_3g.alarm_3g_mms_user_name,0,CRED_3G_MMS_USER_STR);/*!<  MMS user name */
			memset(alarm_system_info_global_ptr->alarm_3g.alarm_3g_mms_pwd,0,CRED_3G_MMS_PWD_STR);/*!<  MMS pwd*/
			memset(alarm_system_info_global_ptr->alarm_3g.alarm_3g_mms_url_mmsc,0,CRED_3G_MMS_URL_MMSC_STR);/*!<  MMS pwd*/
			memset(alarm_system_info_global_ptr->alarm_3g.alarm_3g_mms_proxy_ip,0,MAX_IP_ADDRESS_STR);/*!<  MMS IP proxy*/
			alarm_system_info_global_ptr->alarm_3g.alarm_3g_mms_port=0;/*!<  MMS port number*/
			/*!< 3g Module data setting */
			alarm_system_info_global_ptr->alarm_3g.alarm_3g_data_auth_flag=CRED_3G_TCP_AUTH_TYPE_NONE;/*!<  Data access point*/
			memset(alarm_system_info_global_ptr->alarm_3g.alarm_3g_data_ap,0,CRED_3G_MMS_AP_STR);/*!<  Data access point*/
			memset(alarm_system_info_global_ptr->alarm_3g.alarm_3g_data_user_name,0,CRED_3G_MMS_USER_STR);/*!<  Data user name */
			memset(alarm_system_info_global_ptr->alarm_3g.alarm_3g_data_pwd,0,CRED_3G_MMS_PWD_STR);/*!<  Data pwd*/
			memset(alarm_system_info_global_ptr->alarm_3g.alarm_3g_data_ip,0,MAX_IP_ADDRESS_STR);/*!<  Data IP */
			memset(alarm_system_info_global_ptr->alarm_3g.alarm_3g_data_port,0,MAX_IP_PORT_STR);/*!<  Data Port */
			/*!< email setting */
			memset(alarm_system_info_global_ptr->alarm_email.alarm_email_server,0,CRED_EMAIL_SERVER_STR);/*!<  Email access point*/
			memset(alarm_system_info_global_ptr->alarm_email.alarm_email_user_name,0,CRED_EMAIL_USER_STR);/*!<  Email user name */
			memset(alarm_system_info_global_ptr->alarm_email.alarm_email_pwd,0,CRED_EMAIL_PWD_STR);/*!<  Email pwd*/
			alarm_system_info_global_ptr->alarm_email.alarm_email_port =0;/*!<  Email port number*/
			/*!< back up copy setting */
			memcpy(alarm_system_info_global_ptr+1,alarm_system_info_global_ptr,sizeof(alarm_system_info_t));

		fwrite(alarm_system_info_global_ptr, (sizeof(alarm_system_info_t)*(2)), 1, fp_sys_info);
		ALARM_TRACE_INFO("Create SYS info File \n ");
	}
	else
	{
		/*File Exist Read Old Information*/
		fread(alarm_system_info_global_ptr, (sizeof(alarm_system_info_t)*(2)), 1, fp_sys_info);
		ALARM_TRACE_INFO("Read SYS INFO Code Saved  Information \n ");
	}
	/*Close sys info file*/
	fclose(fp_sys_info);

	return CRED_MW_NO_ERROR;
}

/**
 * \fn CRED_MW_Errors_t alarm_check_system_info(void)
 * \brief check alarm system network info.
 *
 * \param : uint32_t *alarm_network_info_ptr
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_check_system_info(void)
{
	FILE *fp_sys_info=NULL;
	int eth_link_status,pwr_int_value,battery_soc;
	/*Open installer Code file:*/
	fp_sys_info = fopen(CRED_SYS_INF_FILE_PATH, "r+");
	if(fp_sys_info == NULL)
	{
		/*File Does Not Exit => Create the File*/
		fp_sys_info = fopen(CRED_SYS_INF_FILE_PATH, "w+");
	}
	/*!<  check ethernet status*/
	ETHERNET_Ethernet_Check_Link(&eth_link_status);
	ALARM_TRACE_INFO("Ethernet link status %d \n ",eth_link_status);
	if(eth_link_status == up){
		alarm_system_info_global_ptr->alarm_network.alarm_ethernet_status=CRED_ETHERNET_OK;	/*!< Ethernet Status : Connected*/
		ALARM_TRACE_INFO("Ethernet link status up \n ");
	}
	else{
		alarm_system_info_global_ptr->alarm_network.alarm_ethernet_status=CRED_ETHERNET_NOK;	/*!< Ethernet Status : Not Connected*/
		ALARM_TRACE_INFO("Ethernet link status down \n ");
	}
	/*!<  check power status*/
	GPIO_GetPinValue(IRQ_POWER_DET_GROUP, IRQ_POWER_DET_PIN, &pwr_int_value);
	ALARM_TRACE_INFO("Power status %d \n ",pwr_int_value);
	if(pwr_int_value){
		alarm_system_info_global_ptr->alarm_power.alarm_power_source_status = CRED_ON;	/*!< Power Status : ON*/
		ALARM_TRACE_INFO("Power status ON \n ");
	}
	else{
		alarm_system_info_global_ptr->alarm_power.alarm_power_source_status = CRED_OFF;	/*!< Power Status : OFF*/
		ALARM_TRACE_INFO("Power status OFF \n ");
	}
	/*!<  check battery status*/
	adc_get_channel_value(2,&battery_soc);
	ALARM_TRACE_INFO("Battery state of charge %d \n ",battery_soc);
	if(battery_soc == 0)
	{
		alarm_system_info_global_ptr->alarm_power.alarm_battery_status  = CRED_BATTERY_NO_BATTERY;
		ALARM_TRACE_INFO("Battery state of charge: CRED_BATTERY_NO_BATTERY \n ");
	}else if(battery_soc < BATTERY_LEVEL1)
	{
		alarm_system_info_global_ptr->alarm_power.alarm_battery_status  = CRED_BATTERY_SIGNAL_LEVEL_1;
		ALARM_TRACE_INFO("Battery state of charge: CRED_BATTERY_SIGNAL_LEVEL_1 \n ");
	}else if(battery_soc < BATTERY_LEVEL2)
	{
		alarm_system_info_global_ptr->alarm_power.alarm_battery_status  = CRED_BATTERY_SIGNAL_LEVEL_2;
		ALARM_TRACE_INFO("Battery state of charge: CRED_BATTERY_SIGNAL_LEVEL_2 \n ");
	}else if(battery_soc < BATTERY_LEVEL3)
	{
		alarm_system_info_global_ptr->alarm_power.alarm_battery_status  = CRED_BATTERY_SIGNAL_LEVEL_3;
		ALARM_TRACE_INFO("Battery state of charge: CRED_BATTERY_SIGNAL_LEVEL_3 \n ");
	}else
	{
		alarm_system_info_global_ptr->alarm_power.alarm_battery_status  = CRED_BATTERY_SIGNAL_LEVEL_4;
		ALARM_TRACE_INFO("Battery state of charge: CRED_BATTERY_SIGNAL_LEVEL_4 \n ");
	}
	/*!<  check wifi status*/
	/*!<  check 3G status*/
	/*Close sys info file*/
	fclose(fp_sys_info);

	return CRED_MW_NO_ERROR;
}
/**
 * \fn CRED_MW_Errors_t alarm_update_system_info(uint32_t *alarm_network_info_ptr)
 * \brief update alarm system network info.
 *
 * \param : uint32_t *alarm_system_info_ptr)
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_update_system_info(uint32_t *alarm_system_info_ptr)
{
	FILE *fp_sys_info=NULL;

	/*Open arm_option  file:*/
	fp_sys_info = fopen(CRED_SYS_INF_FILE_PATH, "r+");
	if(fp_sys_info == NULL)
	{
		/*File Does Not Exit => Create the File*/
		fp_sys_info = fopen(CRED_SYS_INF_FILE_PATH, "w+");
	}

	/*!< back up copy setting */
	memcpy(alarm_system_info_global_ptr+1,alarm_system_info_global_ptr,sizeof(alarm_system_info_t));
	fwrite(alarm_system_info_global_ptr, (sizeof(alarm_system_info_t)*(2)), 1, fp_sys_info);
	*alarm_system_info_ptr = alarm_system_info_global_ptr;

	/*Close sys info file*/
	fclose(fp_sys_info);

	return CRED_MW_NO_ERROR;
}
/**
 * \fn CRED_MW_Errors_t alarm_update_system_info(uint32_t *alarm_network_info_ptr)
 * \brief update alarm system network info.
 *
 * \param : uint32_t *alarm_system_info_ptr)
 * \return No parameter.
 */
CRED_MW_Errors_t alarm_save_system_info(alarm_system_info_t *alarm_system_info_ptr)
{
	FILE *fp_sys_info=NULL;

	/*Open arm_option  file:*/
	fp_sys_info = fopen(CRED_SYS_INF_FILE_PATH, "r+");
	if(fp_sys_info == NULL)
	{
		/*File Does Not Exit => Create the File*/
		fp_sys_info = fopen(CRED_SYS_INF_FILE_PATH, "w+");
	}

	/*!< back up copy setting */
	memcpy(alarm_system_info_global_ptr,alarm_system_info_ptr,sizeof(alarm_system_info_t));
	memcpy(alarm_system_info_global_ptr+1,alarm_system_info_global_ptr,sizeof(alarm_system_info_t));
	fwrite(alarm_system_info_global_ptr, (sizeof(alarm_system_info_t)*(2)), 1, fp_sys_info);

	/*Close sys info file*/	
	fclose(fp_sys_info);
	
	return CRED_MW_NO_ERROR;
}
/*! \fn CRED_Errors_t alarm_get_system_time(struct RTC_Alarm rtc_time)
    \brief get the current RTC time.
    \param struct RTC_Alarm *rtc_time.
    \param no .
*/

CRED_Errors_t alarm_get_system_time(struct RTC_Alarm *rtc_time)
{

	CRED_Errors_t err = CRED_NO_ERROR;

	err = rtc_get_time(rtc_time);

	return err;
}
/*! \fn CRED_Errors_t alarm_set_system_time(struct RTC_Alarm rtc_time)
    \brief Set the new RTC time.
    \param rtc_fd The name of the descriptor.
    \param rtc_time the new params to set (date, time) .
*/

CRED_Errors_t alarm_set_system_time(struct RTC_Alarm rtc_time)
{

	CRED_Errors_t err = CRED_NO_ERROR;

	err = rtc_set_time_only(rtc_time);

	return err;
}
/*! \fn CRED_Errors_t alarm_set_system_date(struct RTC_Alarm rtc_time)
    \brief Set the new RTC time.
    \param rtc_fd The name of the descriptor.
    \param rtc_time the new params to set (date, time) .
*/
CRED_Errors_t alarm_set_system_date(struct RTC_Alarm rtc_time)
{

	CRED_Errors_t err = CRED_NO_ERROR;

	err = rtc_set_date_only(rtc_time);

	return err;
}

/*! \fn CRED_Errors_t alarm_set_system_date_format(int *date_format_app)
    \brief Set the new Date format .
    \param int *date_format_app date fromat pointer.
 * \return No parameter.
*/
CRED_Errors_t alarm_set_system_date_format(int *date_format_app)
{
	CRED_Errors_t err = CRED_NO_ERROR;
	int date_format_tmp;
	
	if(date_format_app == NULL )
	{
		ALARM_TRACE_ERROR("date_format_app null pointer \n");
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	
	date_format_tmp = *date_format_app;
	if((date_format_tmp != CRED_DATE_FORMAT_DDMMYY ) && (date_format_tmp != CRED_DATE_FORMAT_MMDDYY ))
	{
		ALARM_TRACE_ERROR("Date format %d Not supported\n",date_format_tmp);
		*date_format_app = CRED_DATE_FORMAT_NOT_USED;
		return err;
	}
	alarm_system_info_global_ptr->alarm_genral.date_format = date_format_tmp;
	alarm_save_system_info(alarm_system_info_global_ptr);

	return err;
}

/*! \fn CRED_Errors_t alarm_set_system_date_format(int *date_format_app)
    \brief Set the new Date format .
    \param int *date_format_app date fromat pointer.
 * \return No parameter.
*/
CRED_Errors_t alarm_get_system_date_format(int *date_format_app)
{
	CRED_Errors_t err = CRED_NO_ERROR;

	if(date_format_app == NULL )
	{
		ALARM_TRACE_ERROR("date_format_app null pointer \n");
		return CRED_MW_ERROR_BAD_PARAMETER;
	}

	*date_format_app = alarm_system_info_global_ptr->alarm_genral.date_format;

	return err;
}

/*! \fn CRED_Errors_t alarm_set_system_time_zone(int *time_zone_app)
    \brief Set the new time zone.
    \param int *time_zone_app time zone pointer.
 * \return No parameter.
*/
CRED_Errors_t alarm_set_system_time_zone(int *time_zone_app)
{
	CRED_Errors_t err = CRED_NO_ERROR;
	int time_zone_tmp;
	
	if(time_zone_app == NULL )
	{
		ALARM_TRACE_ERROR("time_zone_app null pointer \n");
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	
	time_zone_tmp = *time_zone_app;
	if((time_zone_tmp >= CRED_TIME_ZONE_LAST ) || (time_zone_tmp <= CRED_TIME_ZONE_NOT_USED ))
	{
		ALARM_TRACE_ERROR("Time Zonet %d Not supported\n",time_zone_tmp);
		*time_zone_app = CRED_TIME_ZONE_NOT_USED;
		return err;
	}
	alarm_system_info_global_ptr->alarm_genral.time_zone = time_zone_tmp;
	alarm_save_system_info(alarm_system_info_global_ptr);

	return err;
}

/*! \fn CRED_Errors_t alarm_set_system_time_zone(int *time_zone_app)
    \brief Set the new time zone.
    \param int *time_zone_app time zone pointer.
 * \return No parameter.
*/
CRED_Errors_t alarm_get_system_time_zone(int *time_zone_app)
{
	CRED_Errors_t err = CRED_NO_ERROR;

	if(time_zone_app == NULL )
	{
		ALARM_TRACE_ERROR("time_zone_app null pointer \n");
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	*time_zone_app = alarm_system_info_global_ptr->alarm_genral.time_zone;
	return err;
}

/*! \fn CRED_Errors_t alarm_get_ntp_server_status(int *ntp_server_status)
    \brief get the ntp server status.
    \param int *ntp_server_status ntp status pointer.
 * \return No parameter.
*/
CRED_Errors_t alarm_get_ntp_server_status(int *ntp_server_status)
{
	CRED_Errors_t err = CRED_NO_ERROR;

	if(ntp_server_status == NULL )
	{
		ALARM_TRACE_ERROR("ntp_server_status null pointer \n");
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	*ntp_server_status = alarm_system_info_global_ptr->alarm_genral.ntp_server_enable;
	return err;
}

/*! \fn CRED_Errors_t alarm_enable_disable_ntp(int *ntp_server_status)
    \brief Enable/Disable NTP server.
    \param int *ntp_server_status ENABLE/DISABLE pointer.
 * \return No parameter.
*/
CRED_Errors_t alarm_enable_disable_ntp(int *ntp_server_status)
{
	CRED_Errors_t err = CRED_NO_ERROR;
	int ntp_server_status_tmp;
	
	if(ntp_server_status == NULL )
	{
		ALARM_TRACE_ERROR("ntp_server_status null pointer \n");
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	
	ntp_server_status_tmp = *ntp_server_status;
	if(ntp_server_status_tmp == CRED_ENABBLE)
	{
		alarm_system_info_global_ptr->alarm_genral.ntp_server_enable = CRED_ENABBLE;
		
	}else if(ntp_server_status_tmp == CRED_DISABLE)
	{
		alarm_system_info_global_ptr->alarm_genral.ntp_server_enable = CRED_DISABLE;
	}else
	{
		ALARM_TRACE_ERROR("NTP Status %d Not supported\n",ntp_server_status_tmp);
		*ntp_server_status = CRED_ENABLE_NOT_USED;
		return err;
	}

	alarm_save_system_info(alarm_system_info_global_ptr);
	return err;
}
/*! \fn CRED_Errors_t alarm_set_system_language(int *language_app)
    \brief Set the new time zone.
    \param int *language_app time zone pointer.
 * \return No parameter.
*/
CRED_Errors_t alarm_set_system_language(int *language_app)
{
	CRED_Errors_t err = CRED_NO_ERROR;
	int language_tmp;
	
	if(language_app == NULL )
	{
		ALARM_TRACE_ERROR("language_app null pointer \n");
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	
	language_tmp = *language_app;
	if((language_tmp >= CRED_LANGUAGE_LAST ) || (language_tmp <= CRED_LANGUAGE_NOT_USED ))
	{
		ALARM_TRACE_ERROR("Language %d Not supported\n",language_tmp);
		*language_app = CRED_LANGUAGE_NOT_USED;
		return err;
	}
	alarm_system_info_global_ptr->alarm_genral.language = language_tmp;
	alarm_save_system_info(alarm_system_info_global_ptr);

	return err;
}

/*! \fn CRED_Errors_t alarm_set_system_language(int *language_app)
    \brief Set the new time zone.
    \param int *language_app time zone pointer.
 * \return No parameter.
*/
CRED_Errors_t alarm_get_system_language(int *language_app)
{
	CRED_Errors_t err = CRED_NO_ERROR;

	if(language_app == NULL )
	{
		ALARM_TRACE_ERROR("language_app null pointer \n");
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	*language_app = alarm_system_info_global_ptr->alarm_genral.language;
	return err;
}
/*! \fn CRED_Errors_t alarm_set_system_daylight_saving_time(int *daylight_saving_time_app)
    \brief Set the new time zone.
    \param int *daylight_saving_time_app time zone pointer.
 * \return No parameter.
*/
CRED_Errors_t alarm_set_system_daylight_saving_time(int *daylight_saving_time_app)
{
	CRED_Errors_t err = CRED_NO_ERROR;
	int daylight_saving_time_tmp;
	
	if(daylight_saving_time_app == NULL )
	{
		ALARM_TRACE_ERROR("daylight_saving_time_app null pointer \n");
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	
	daylight_saving_time_tmp = *daylight_saving_time_app;
	if((daylight_saving_time_tmp >= CRED_DAYLIGHT_SAVING_TIME_LAST ) || (daylight_saving_time_tmp <= CRED_DAYLIGHT_SAVING_TIME_NOT_USED ))
	{
		ALARM_TRACE_ERROR("daylight_saving_time_tmp %d Not supported\n",daylight_saving_time_tmp);
		*daylight_saving_time_app = CRED_DAYLIGHT_SAVING_TIME_NOT_USED;
		return err;
	}
	alarm_system_info_global_ptr->alarm_genral.daylight_saving_time = daylight_saving_time_tmp;
	alarm_save_system_info(alarm_system_info_global_ptr);

	return err;
}

/*! \fn CRED_Errors_t alarm_set_system_daylight_saving_time(int *daylight_saving_time_app)
    \brief Set the new time zone.
    \param int *daylight_saving_time_app time zone pointer.
 * \return No parameter.
*/
CRED_Errors_t alarm_get_system_daylight_saving_time(int *daylight_saving_time_app)
{
	CRED_Errors_t err = CRED_NO_ERROR;

	if(daylight_saving_time_app == NULL )
	{
		ALARM_TRACE_ERROR("daylight_saving_time_app null pointer \n");
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	*daylight_saving_time_app = alarm_system_info_global_ptr->alarm_genral.daylight_saving_time;
	return err;
}
/*! \fn CRED_Errors_t alarm_get_mask_tamper_status(int *mask_tamper_status)
    \brief get the ntp server status.
    \param int *mask_tamper_status ntp status pointer.
 * \return No parameter.
*/
CRED_Errors_t alarm_get_mask_tamper_status(int *mask_tamper_status)
{
	CRED_Errors_t err = CRED_NO_ERROR;

	if(mask_tamper_status == NULL )
	{
		ALARM_TRACE_ERROR("mask_tamper_status null pointer \n");
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	*mask_tamper_status = arm_option_global_ptr->mask_tamper ;
	return err;
}

/*! \fn CRED_Errors_t alarm_enable_disable_mask_tamper(int *mask_tamper_status)
    \brief Enable/Disable NTP server.
    \param int *mask_tamper_status ENABLE/DISABLE pointer.
 * \return No parameter.
*/
CRED_Errors_t alarm_enable_disable_mask_tamper(int *mask_tamper_status)
{
	CRED_Errors_t err = CRED_NO_ERROR;
	int mask_tamper_status_tmp;
	
	if(mask_tamper_status == NULL )
	{
		ALARM_TRACE_ERROR("mask_tamper_status null pointer \n");
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	
	mask_tamper_status_tmp = *mask_tamper_status;
	if(mask_tamper_status_tmp == CRED_ENABBLE)
	{
		arm_option_global_ptr->mask_tamper = CRED_ENABBLE;
		
	}else if(mask_tamper_status_tmp == CRED_DISABLE)
	{
		arm_option_global_ptr->mask_tamper = CRED_DISABLE;
	}else
	{
		ALARM_TRACE_ERROR("NTP Status %d Not supported\n",mask_tamper_status_tmp);
		*mask_tamper_status = CRED_ENABLE_NOT_USED;
		return err;
	}

	alarm_save_arm_option(arm_option_global_ptr);
	return err;
}

/*! \fn CRED_Errors_t alarm_technical_reset(int *technical_reset_status)  
    \brief installer technical reset.
    \param int *technical_reset status pointer.
 * \return No parameter.
*/
CRED_Errors_t aalarm_technical_reset(int *technical_reset_status)  
{
	CRED_Errors_t err = CRED_NO_ERROR;
		
	if(technical_reset_status == NULL )
	{
		ALARM_TRACE_ERROR("technical_reset_status null pointer \n");
		return CRED_MW_ERROR_BAD_PARAMETER;
	}
	
	
	return err;
}
