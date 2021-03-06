#include "App.hpp"
#include "DefaultAppConfig.h"
#include <FS.h>
#include <Ticker.h>
#include <DisplayHandler.hpp>

App app;
AppConfig appcfg;

Ticker appTicker;
volatile unsigned long secCounter;

void appTickerCallback() {
  secCounter++;
}

App::App() {
  initialized = true;
  doSystemRestart = false;
  secCounter = 0;

  strncpy( appcfg.wifi_ssid, DEFAULT_WIFI_SSID, sizeof(appcfg.wifi_ssid) );
  strncpy( appcfg.wifi_password, DEFAULT_WIFI_PASSWORD, sizeof(appcfg.wifi_password) );
  appcfg.wifi_mode = DEFAULT_WIFI_MODE;

  strncpy( appcfg.ota_hostname, DEFAULT_OTA_HOSTNAME, sizeof(appcfg.ota_hostname) );
  strncpy( appcfg.ota_password, DEFAULT_OTA_PASSWORD, sizeof(appcfg.ota_password) );

  strncpy( appcfg.admin_password, DEFAULT_ADMIN_PASSWORD, sizeof(appcfg.admin_password) );

  appcfg.ohab_enabled = DEFAULT_OHAB_ENABLED;
  appcfg.ohab_version = DEFAULT_OHAB_VERSION;
  strncpy( appcfg.ohab_host, DEFAULT_OHAB_HOST, sizeof(appcfg.ohab_host) );
  appcfg.ohab_port = DEFAULT_OHAB_PORT;
  appcfg.ohab_useauth = DEFAULT_OHAB_USEAUTH;
  strncpy( appcfg.ohab_user, DEFAULT_OHAB_USER, sizeof(appcfg.ohab_user) );
  strncpy( appcfg.ohab_password, DEFAULT_OHAB_PASSWORD, sizeof(appcfg.ohab_password) );
  strncpy( appcfg.ohab_itemname_temp, DEFAULT_OHAB_ITEMNAME_TEMP, sizeof(appcfg.ohab_itemname_temp) );
  strncpy( appcfg.ohab_itemname_humi, DEFAULT_OHAB_ITEMNAME_HUMI, sizeof(appcfg.ohab_itemname_humi) );

  appcfg.mqtt_enabled = DEFAULT_MQTT_ENABLED;
  strncpy( appcfg.mqtt_clientid, DEFAULT_MQTT_CLIENTID, sizeof(appcfg.mqtt_clientid) );
  strncpy( appcfg.mqtt_host, DEFAULT_MQTT_HOST, sizeof(appcfg.mqtt_host) );
  appcfg.mqtt_port = DEFAULT_MQTT_PORT;
  appcfg.mqtt_useauth = DEFAULT_MQTT_USEAUTH;
  strncpy( appcfg.mqtt_user, DEFAULT_MQTT_USER, sizeof(appcfg.mqtt_user) );
  strncpy( appcfg.mqtt_password, DEFAULT_MQTT_PASSWORD, sizeof(appcfg.mqtt_password) );
  strncpy( appcfg.mqtt_topic_out, DEFAULT_MQTT_TOPIC_OUT, sizeof(appcfg.mqtt_topic_out) );
  strncpy( appcfg.ntp_host, DEFAULT_NTP_HOST, sizeof(appcfg.ntp_host) );
}

void App::restartSystem() {
  displayHandler.showResetSystem();
  // watchdogTicker.detach();
  ESP.eraseConfig();
  LOG0( "*** restarting system ***\n" );
  delay( 2000 );
  ESP.restart();
  delay( 2000 );
  ESP.reset();
}

void App::setup() {
  Serial.begin(115200);

  doSystemRestart = false;
  appTicker.attach( 1.0, appTickerCallback );

  displayHandler.showBooting();

  delay( 3000 );

  Serial.println();
  Serial.println("\n\n");
  Serial.println("\n\n");
  Serial.println(F(APP_NAME ", Version " APP_VERSION ", by " APP_AUTHOR));
  Serial.println( "Build date: " __DATE__ " " __TIME__  );
  Serial.printf("appcfg file size: %d bytes\n\n", sizeof( appcfg ));

  /*
  if ( digitalRead(POWER_BUTTON) == false )
  {

    Serial.println();
    LOG0("*** RESET appcfguration ***\n");
    Serial.println();

    for( int i=0; i<15; i++)
    {
      digitalWrite( WIFI_LED, 1 );
      delay( 100 );
      digitalWrite( WIFI_LED, 0 );
      delay( 100 );
    }

    digitalWrite( WIFI_LED, 1 );

    ESP.eraseConfig();

    if (SPIFFS.begin())
    {
      LOG0("File system format started...\n");
      SPIFFS.format();
      LOG0("File system format finished.\n");
      SPIFFS.end();
    }

    digitalWrite( WIFI_LED, 0 );
    restartSystem();
  }
  */

  ESP.eraseConfig();
  loadConfig();
}


void App::loadConfig() {
  if (!SPIFFS.begin()) {
    LOG0("ERROR: Failed to mount file system");
  } else {
    if( SPIFFS.exists(APP_CONFIG_FILE)) {
      displayHandler.showMessage("Loading\nConfig");
      File configFile = SPIFFS.open( APP_CONFIG_FILE, "r");

      if (!configFile) {
        LOG1("ERROR: file %s not found.\n", APP_CONFIG_FILE );
      } else {
        LOG1("Loading appcfguration from %s file...\n", APP_CONFIG_FILE );

        if( configFile.size() != sizeof( appcfg )) {
          Serial.printf( "ERROR: %s file size not match appcfg structure %d != %d bytes.\n", APP_CONFIG_FILE, configFile.size(), sizeof( appcfg ));
        } else {
          int bytesRead = configFile.readBytes((char *)&appcfg, sizeof( appcfg ));
          LOG1( "%d bytes read from appcfg file.\n", bytesRead );
          configFile.close();
        }
      }
    } else {
      LOG0( "WARNING: appcfg file " APP_CONFIG_FILE " does not exist. Using default appcfg.\n" );
    }
    SPIFFS.end();
  }
}

void App::writeConfig() {
  if (!SPIFFS.begin()) {
    LOG0("ERROR: Failed to mount file system");
  } else {
    LOG1("writing file %s.\n", APP_CONFIG_FILE );

    File configFile = SPIFFS.open( APP_CONFIG_FILE, "w");

    if (!configFile) {
      LOG1("ERROR: Failed to open appcfg file %s for writing.\n", APP_CONFIG_FILE );
      return;
    }

    int length = configFile.write((const uint8_t *)&appcfg, sizeof( appcfg ));
    LOG1("%d bytes written to appcfg file.\n", length );

    configFile.close();

    FSInfo fs_info;
    SPIFFS.info(fs_info);

    Serial.printf( "\n--- SPIFFS Info ---\ntotal bytes = %d\n", fs_info.totalBytes );
    Serial.printf( "used bytes = %d\n", fs_info.usedBytes );
    Serial.printf( "block size = %d\n", fs_info.blockSize );
    Serial.printf( "page size = %d\n", fs_info.pageSize );
    Serial.printf( "max open files = %d\n", fs_info.maxOpenFiles );
    Serial.printf( "max path length = %d\n", fs_info.maxPathLength );

    SPIFFS.end();
  }
}

void App::printConfig() {
  Serial.println();
  Serial.println( "--- App appcfguration -----------------------------------" );
  Serial.println( "  Security:" );
  Serial.printf(  "    Admin password: %s\n", appcfg.admin_password );
  Serial.println( "\n  WiFi:" );
  Serial.printf(  "    SSID: %s\n", appcfg.wifi_ssid );
  Serial.printf(  "    Password: %s\n", appcfg.wifi_password );
  Serial.printf(  "    Mode: %s\n", ( appcfg.wifi_mode == 1 ) ? "Station" : "Access Point" );
  Serial.println( "\n  OTA:" );
  Serial.printf(  "    Hostname: %s\n", appcfg.ota_hostname );
  Serial.printf(  "    Password: %s\n", appcfg.ota_password );
  Serial.println( "\n  OpenHAB:" );
  Serial.printf(  "    Enabled: %s\n", (appcfg.ohab_enabled ? "true" : "false" ));
  Serial.printf(  "    Version: %d\n", appcfg.ohab_version );
  Serial.printf(  "    Host: %s\n", appcfg.ohab_host );
  Serial.printf(  "    Port: %d\n", appcfg.ohab_port );
  Serial.printf(  "    Use Auth: %s\n", (appcfg.ohab_useauth ? "true" : "false" ));
  Serial.printf(  "    User: %s\n", appcfg.ohab_user );
  Serial.printf(  "    Password: %s\n", appcfg.ohab_password );
  Serial.printf(  "    Itemname Temperature: %s\n", appcfg.ohab_itemname_temp );
  Serial.printf(  "    Itemname Humidity: %s\n", appcfg.ohab_itemname_humi );
  Serial.println( "\n  MQTT:" );
  Serial.printf(  "    Enabled: %s\n", (appcfg.mqtt_enabled ? "true" : "false" ));
  Serial.printf(  "    Client ID: %s\n", appcfg.mqtt_clientid );
  Serial.printf(  "    Host: %s\n", appcfg.mqtt_host );
  Serial.printf(  "    Port: %d\n", appcfg.mqtt_port );
  Serial.printf(  "    Use Auth: %s\n", (appcfg.mqtt_useauth ? "true" : "false" ));
  Serial.printf(  "    User: %s\n", appcfg.mqtt_user );
  Serial.printf(  "    Password: %s\n", appcfg.mqtt_password );
  Serial.printf(  "    Out Topic: %s\n", appcfg.mqtt_topic_out );
  Serial.println( "\n  NTP:" );
  Serial.printf(  "    Host: %s\n", appcfg.ntp_host );
  Serial.println( "---------------------------------------------------------" );
  Serial.println();
}

void App::delayedSystemRestart() {
  doSystemRestart = true;
  systemRestartTimestamp = millis();
  LOG0("*** delayedSystemRestart ***\n");
}

void App::handle() {
  if( doSystemRestart && ( millis() - systemRestartTimestamp ) > 5000l ) {
    LOG0("*** doSystemRestart ***\n");
    writeConfig();
    restartSystem();
  }
}

unsigned long App::secTimestamp() {
  return secCounter;
}
