#include <stdio.h>
#include <curl/curl.h>
#include <string.h>
#include "3g_cred.h"
#include "uart_cred.h"
#include "middleware_cred.h"
#include "notification_cred.h" 
#include "telephony_cred.h"
#include "push_notif.h"

extern unsigned int token_is_got_global;
extern char* token_char_pt_global;
int main(void)
{
	CURL *curl;
	CURLcode res;
	struct curl_slist *list = NULL;
	char postfields[1024];
//	char *token="cGVPvhfbYuk:APA91bFzwtDGftKFxgz-yBMkIW16-kO5t3HT0AoPCQcpE_1JFnmd_fPOlz3Xo-i_fUWqd4ct2nuBdVCf3cEhhfE0uFBocSozizsBEGwJmpAsVsQS_RjqmysRiWKNVzgkxEn7VbDT8FBG";
	char *message="hello cred";
	
	
	
	struct Ns_Notify_Str  Ts_notify_S;
	struct Ns_Notify_Str  Ts_notify_S_InEvents; 
	CRED_Errors_t err = CRED_NO_ERROR;  
	
	if(TELEPHONY_NOTIFICATION_Init(&Ts_notify_S)!=CRED_NO_ERROR)
	{
		printf("fail to init telephony notification system \n");
		return -1;
	}   
	m3g_init();   
	
	M3G_Send_SMS("55940854","GET TOKEN");
	token_is_got_global=0;
	while(token_is_got_global==0)
	{
		usleep(1000);
	}
	token_char_pt_global=strchr(token_char_pt_global,':');
	token_char_pt_global++;
	printf("----------------------------------\n");
	printf("token_char_global:%s\n",token_char_pt_global);
	printf("----------------------------------\n");
	system("udhcpc");
	sprintf(postfields, "{\"registration_ids\":[\"%s\"],\"data\":{\"type1\":\"%s\"}}", token_char_pt_global, message);
	curl = curl_easy_init();
	if(curl) {


	    curl_easy_setopt(curl, CURLOPT_URL, "https://gcm-http.googleapis.com/gcm/send");


	 	list = curl_slist_append(list, "Authorization: key=AIzaSyDTJTy1RRz5Vj5MX0m82b0BnoBbhszcUJY");
		list = curl_slist_append(list, "Content-Type: application/json");

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
	   // curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header); 
	    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfields);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

	    res = curl_easy_perform(curl);
	    /* Check for errors */ 
	  
	    if(res != CURLE_OK)
	      fprintf(stderr, "curl_easy_perform() failed: %d\n",
	              res);
	 
	    /* always cleanup */ 
	    curl_easy_cleanup(curl);
	}
}
 //curl --header "Authorization: key=AIzaSyDTJTy1RRz5Vj5MX0m82b0BnoBbhszcUJY" --header "Content-Type: application/json" https://gcm-http.googleapis.com/gcm/send -d "{\"registration_ids\":[\"cGVPvhfbYuk:APA91bFYHFJjEGAe_8rnB2NIhiZN4uJJmbBibclowQbsAT3CNnSAP73bYNVYse_i-MIDKeQhOlQa76e14N4sMARu3F-0SnA4ZSN-2XlYQ-DndMDSaevG8BMft0c94i5PW_Xf2B1Inr7x\"],\"data\":{\"type1\":\"alert_en_C\"}}"
