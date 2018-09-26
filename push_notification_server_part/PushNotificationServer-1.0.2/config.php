<?php
/**
 * Database config variables
 */
define("DB_HOST", "localhost");
define("DB_USER", "amin");			//fill with value
define("DB_PASSWORD", "azerty");		//fill with value
define("DB_DATABASE", "gcm");		//fill with value

/*
 * Google API Key
 */
define("GOOGLE_API_KEY", "AIzaSyAqxDvz7PTwtPVI7BEQuElc5qGGlpYCAGs"); // Place your Google API Key
//define('GOOGLE_API_URL','https://android.googleapis.com/gcm/send'); //deprecated
define("GOOGLE_API_URL","https://gcm-http.googleapis.com/gcm/send");


//define("BASE_URL", "https://neurobin.org/"); //optional
define("PWD", "./");
// define("PWP", "https://neurobin.org/api/android/gcm/gcm-server-demo/"); //optional

?>
