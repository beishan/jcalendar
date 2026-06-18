#include <Arduino.h>
#include <ArduinoJson.h>

#include <WiFiManager.h>
#include <WebServer.h>

#include "esp_sleep.h"

#include <wiring.h>

#include "battery.h"

#include "led.h"
#include "_sntp.h"
#include "weather.h"
#include "screen_ink.h"
#include "_preference.h"

#include "version.h"

#include "OneButton.h"
OneButton button(KEY_M, true);

void IRAM_ATTR checkTicks() {
    button.tick();
}

WiFiManager wm;
WiFiManagerParameter para_qweather_host("qweather_host", "和风天气Host", "", 64); //     和风天气key
WiFiManagerParameter para_qweather_key("qweather_key", "和风天气API Key", "", 32); //     和风天气key
// const char* test_html = "<br/><label for='test'>天气模式</label><br/><input type='radio' name='test' value='0' checked> 每日天气test </input><input type='radio' name='test' value='1'> 实时天气test</input>";
// WiFiManagerParameter para_test(test_html);
WiFiManagerParameter para_qweather_type("qweather_type", "天气类型（0:每日天气，1:实时天气）", "0", 2, "pattern='\\[0-1]{1}'"); //     城市code
WiFiManagerParameter para_qweather_location("qweather_loc", "位置ID", "", 64); //     城市code
WiFiManagerParameter para_cd_day_label("cd_day_label", "倒数日（4字以内）", "", 10); //     倒数日
WiFiManagerParameter para_cd_day_date("cd_day_date", "日期（yyyyMMdd）", "", 8, "pattern='\\d{8}'"); //     城市code
WiFiManagerParameter para_tag_days("tag_days", "日期Tag（yyyyMMddx，详见README）", "", 30); //     日期Tag
WiFiManagerParameter para_si_week_1st("si_week_1st", "每周起始（0:周日，1:周一）", "0", 2, "pattern='\\[0-1]{1}'"); //     每周第一天
WiFiManagerParameter para_study_schedule("study_schedule", "课程表", "0", 4000, "pattern='\\[0-9]{3}[;]$'"); //     每周第一天

// 局域网配置 Web 服务
WebServer lan_server(80);

void handleRoot() {
    Preferences pref;
    pref.begin(PREF_NAMESPACE);
    String qHost = pref.getString(PREF_QWEATHER_HOST, "api.qweather.com");
    String qKey = pref.getString(PREF_QWEATHER_KEY, "");
    String qType = pref.getString(PREF_QWEATHER_TYPE, "0");
    String qLoc = pref.getString(PREF_QWEATHER_LOC, "");
    String cdLabel = pref.getString(PREF_CD_DAY_LABLE, "");
    String cdDate = pref.getString(PREF_CD_DAY_DATE, "");
    String tagDays = pref.getString(PREF_TAG_DAYS, "");
    String week1st = pref.getString(PREF_SI_WEEK_1ST, "0");
    String studySchedule = pref.getString(PREF_STUDY_SCHEDULE, "");
    pref.end();

    // 天气配置状态
    String weatherStatus = (qKey.length() > 0 && qLoc.length() > 0)
        ? "<span style='color:#4CAF50;font-weight:bold'>✓ 已配置</span>"
        : "<span style='color:#f44336;font-weight:bold'>✗ 未配置（请填写Key和位置ID）</span>";

    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
        "<meta name='viewport' content='width=device-width,initial-scale=1'>"
        "<title>J-Calendar</title>"
        "<style>"
        "body{font-family:Arial,sans-serif;max-width:480px;margin:20px auto;padding:0 15px;background:#f5f5f5}"
        "h1{color:#333;text-align:center}"
        ".card{background:#fff;border-radius:8px;padding:15px;margin:10px 0;box-shadow:0 1px 3px rgba(0,0,0,0.1)}"
        "label{display:block;font-weight:bold;margin:8px 0 4px;color:#555}"
        "input,select{width:100%;padding:8px;border:1px solid #ddd;border-radius:4px;box-sizing:border-box}"
        "button{width:100%;padding:12px;background:#4CAF50;color:#fff;border:none;border-radius:4px;font-size:16px;margin-top:15px;cursor:pointer}"
        "button:hover{background:#45a049}"
        ".info{text-align:center;color:#888;font-size:12px;margin-top:20px}"
        ".status{margin:4px 0 8px;font-size:13px}"
        "</style></head><body>"
        "<h1>J-Calendar</h1>"
        "<form action='/save' method='POST'>"
        "<div class='card'>"
        "<label>天气配置 " + weatherStatus + "</label>"
        "<label>天气API Host</label><input name='qweather_host' value='" + qHost + "'>"
        "<label>天气API Key</label><input name='qweather_key' value='" + qKey + "' placeholder='请输入和风天气API Key'>"
        "<label>天气类型</label><select name='qweather_type'>"
        "<option value='0'" + String(qType == "0" ? " selected" : "") + ">每日天气</option>"
        "<option value='1'" + String(qType == "1" ? " selected" : "") + ">实时天气</option>"
        "</select>"
        "<label>位置</label>"
        "<select name='qweather_loc' id='qweather_loc' onchange='document.getElementById(\"custom_loc\").style.display=this.value==\"custom\"?\"block\":\"none\"'>"
        "<option value=''" + String(qLoc == "" ? " selected" : "") + ">请选择城市</option>"
        "<option value='101010100'" + String(qLoc == "101010100" ? " selected" : "") + ">北京</option>"
        "<option value='101020100'" + String(qLoc == "101020100" ? " selected" : "") + ">上海</option>"
        "<option value='101280101'" + String(qLoc == "101280101" ? " selected" : "") + ">广州</option>"
        "<option value='101280601'" + String(qLoc == "101280601" ? " selected" : "") + ">深圳</option>"
        "<option value='101210101'" + String(qLoc == "101210101" ? " selected" : "") + ">杭州</option>"
        "<option value='101270101'" + String(qLoc == "101270101" ? " selected" : "") + ">成都</option>"
        "<option value='101200101'" + String(qLoc == "101200101" ? " selected" : "") + ">武汉</option>"
        "<option value='101190101'" + String(qLoc == "101190101" ? " selected" : "") + ">南京</option>"
        "<option value='101040100'" + String(qLoc == "101040100" ? " selected" : "") + ">重庆</option>"
        "<option value='101110101'" + String(qLoc == "101110101" ? " selected" : "") + ">西安</option>"
        "<option value='101030100'" + String(qLoc == "101030100" ? " selected" : "") + ">天津</option>"
        "<option value='101190401'" + String(qLoc == "101190401" ? " selected" : "") + ">苏州</option>"
        "<option value='101180101'" + String(qLoc == "101180101" ? " selected" : "") + ">郑州</option>"
        "<option value='101250101'" + String(qLoc == "101250101" ? " selected" : "") + ">长沙</option>"
        "<option value='101120201'" + String(qLoc == "101120201" ? " selected" : "") + ">青岛</option>"
        "<option value='101070201'" + String(qLoc == "101070201" ? " selected" : "") + ">大连</option>"
        "<option value='101230201'" + String(qLoc == "101230201" ? " selected" : "") + ">厦门</option>"
        "<option value='101220101'" + String(qLoc == "101220101" ? " selected" : "") + ">合肥</option>"
        "<option value='101120101'" + String(qLoc == "101120101" ? " selected" : "") + ">济南</option>"
        "<option value='101290101'" + String(qLoc == "101290101" ? " selected" : "") + ">昆明</option>"
        "<option value='101050101'" + String(qLoc == "101050101" ? " selected" : "") + ">哈尔滨</option>"
        "<option value='101070101'" + String(qLoc == "101070101" ? " selected" : "") + ">沈阳</option>"
        "<option value='101240101'" + String(qLoc == "101240101" ? " selected" : "") + ">南昌</option>"
        "<option value='101260101'" + String(qLoc == "101260101" ? " selected" : "") + ">贵阳</option>"
        "<option value='101300101'" + String(qLoc == "101300101" ? " selected" : "") + ">南宁</option>"
        "<option value='101310101'" + String(qLoc == "101310101" ? " selected" : "") + ">海口</option>"
        "<option value='101140101'" + String(qLoc == "101140101" ? " selected" : "") + ">拉萨</option>"
        "<option value='101160101'" + String(qLoc == "101160101" ? " selected" : "") + ">兰州</option>"
        "<option value='101150101'" + String(qLoc == "101150101" ? " selected" : "") + ">西宁</option>"
        "<option value='101170101'" + String(qLoc == "101170101" ? " selected" : "") + ">银川</option>"
        "<option value='101130101'" + String(qLoc == "101130101" ? " selected" : "") + ">乌鲁木齐</option>"
        "<option value='custom'" + String(qLoc.length() > 0 && qLoc.toInt() == 0 ? " selected" : "") + ">自定义（输入ID）</option>"
        "</select>"
        "<div id='custom_loc' style='display:" + String(qLoc.length() > 0 && qLoc.toInt() == 0 ? "block" : "none") + ";margin-top:8px'>"
        "<input name='qweather_loc_custom' placeholder='输入位置ID' value='" + qLoc + "'>"
        "</div>"
        "</div>"
        "<div class='card'>"
        "<label>倒数日</label>"
        "<label>倒数日名称</label><input name='cd_day_label' maxlength='4' value='" + cdLabel + "'>"
        "<label>倒数日日期</label><input name='cd_day_date' placeholder='yyyyMMdd' value='" + cdDate + "'>"
        "<label>日期Tag</label><input name='tag_days' placeholder='yyyyMMddx' value='" + tagDays + "'>"
        "</div>"
        "<div class='card'>"
        "<label>显示设置</label>"
        "<label>每周起始</label><select name='si_week_1st'>"
        "<option value='0'" + String(week1st == "0" ? " selected" : "") + ">周日</option>"
        "<option value='1'" + String(week1st == "1" ? " selected" : "") + ">周一</option>"
        "</select>"
        "<label>课程表</label><input name='study_schedule' value='" + studySchedule + "'>"
        "</div>"
        "<button type='submit'>保存配置</button>"
        "</form>"
        "<p class='info'>J-Calendar v" + String(J_VERSION) + "</p>"
        "</body></html>";

    lan_server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    lan_server.send(200, "text/html", html);
}

void handleSave() {
    // 处理位置选择：如果选了"custom"，使用自定义输入的值
    String qLoc = lan_server.arg("qweather_loc");
    if (qLoc == "custom") {
        qLoc = lan_server.arg("qweather_loc_custom");
    }

    Preferences pref;
    pref.begin(PREF_NAMESPACE);
    if (lan_server.hasArg("qweather_host")) pref.putString(PREF_QWEATHER_HOST, lan_server.arg("qweather_host"));
    if (lan_server.hasArg("qweather_key")) pref.putString(PREF_QWEATHER_KEY, lan_server.arg("qweather_key"));
    if (lan_server.hasArg("qweather_type")) pref.putString(PREF_QWEATHER_TYPE, lan_server.arg("qweather_type"));
    if (qLoc.length() > 0) pref.putString(PREF_QWEATHER_LOC, qLoc);
    if (lan_server.hasArg("cd_day_label")) pref.putString(PREF_CD_DAY_LABLE, lan_server.arg("cd_day_label"));
    if (lan_server.hasArg("cd_day_date")) pref.putString(PREF_CD_DAY_DATE, lan_server.arg("cd_day_date"));
    if (lan_server.hasArg("tag_days")) pref.putString(PREF_TAG_DAYS, lan_server.arg("tag_days"));
    if (lan_server.hasArg("si_week_1st")) pref.putString(PREF_SI_WEEK_1ST, lan_server.arg("si_week_1st"));
    if (lan_server.hasArg("study_schedule")) pref.putString(PREF_STUDY_SCHEDULE, lan_server.arg("study_schedule"));

    // 验证保存结果
    String verify_key = pref.getString(PREF_QWEATHER_KEY, "");
    String verify_loc = pref.getString(PREF_QWEATHER_LOC, "");
    pref.end();

    Serial.println("[LAN] Config saved:");
    Serial.printf("  qweather_host: %s\n", lan_server.arg("qweather_host").c_str());
    Serial.printf("  qweather_key: %s (len=%d)\n", lan_server.arg("qweather_key").c_str(), lan_server.arg("qweather_key").length());
    Serial.printf("  qweather_type: %s\n", lan_server.arg("qweather_type").c_str());
    Serial.printf("  qweather_loc (raw): '%s' (len=%d)\n", lan_server.arg("qweather_loc").c_str(), lan_server.arg("qweather_loc").length());
    Serial.printf("  qweather_loc (processed): '%s' (len=%d)\n", qLoc.c_str(), qLoc.length());
    Serial.printf("[LAN] Verify - key: %s, loc: %s\n", verify_key.c_str(), verify_loc.c_str());

    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
        "<meta name='viewport' content='width=device-width,initial-scale=1'>"
        "<title>J-Calendar</title>"
        "<style>body{font-family:Arial,sans-serif;text-align:center;padding:50px}"
        "h2{color:#4CAF50}p{color:#666}</style></head><body>"
        "<h2>配置已保存</h2><p>设备即将重启...</p></body></html>";
    lan_server.send(200, "text/html", html);

    si_info("配置已更新");

    delay(1000);
    ESP.restart();
}

void print_wakeup_reason() {
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0:
        Serial.println("Wakeup caused by external signal using RTC_IO");
        break;
    case ESP_SLEEP_WAKEUP_EXT1:
    {
        Serial.println("Wakeup caused by external signal using RTC_CNTL");
        uint64_t status = esp_sleep_get_ext1_wakeup_status();
        if (status == 0) {
            Serial.println(" *None of the configured pins woke us up");
        } else {
            Serial.print(" *Wakeup pin mask: ");
            Serial.printf("0x%016llX\r\n", status);
            for (int i = 0; i < 64; i++) {
                if ((status >> i) & 0x1) {
                    Serial.printf("  - GPIO%d\r\n", i);
                }
            }
        }
        break;
    }
    case ESP_SLEEP_WAKEUP_TIMER:
        Serial.println("Wakeup caused by timer");
        break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
        Serial.println("Wakeup caused by touchpad");
        break;
    case ESP_SLEEP_WAKEUP_ULP:
        Serial.println("Wakeup caused by ULP program");
        break;
    default:
        Serial.printf("Wakeup was not caused by deep sleep.\r\n");
    }
}

void buttonClick(void* oneButton);
void buttonDoubleClick(void* oneButton);
void buttonLongPressStop(void* oneButton);
void go_sleep();
void saveParamsCallback();
void preSaveParamsCallback();

unsigned long _idle_millis;
unsigned long TIME_TO_SLEEP = 180 * 1000;

bool _wifi_flag = false;
unsigned long _wifi_failed_millis;
bool _wakeup_by_button = false;
unsigned long _screen_done_millis = 0;
bool _new_device_mode = false; // 新设备配网模式
void setup() {
    delay(10);
    Serial.begin(115200);
    Serial.println(".");
    print_wakeup_reason();
    // 判断是否由按键唤醒
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    _wakeup_by_button = (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0);
    Serial.println("\r\n\r\n");
    delay(10);


    Serial.printf("***********************\r\n");
    Serial.printf("      J-Calendar\r\n");
    Serial.printf("    version: %s\r\n", J_VERSION);
    Serial.printf("***********************\r\n\r\n");
    Serial.printf("Copyright © 2022-2025 JADE Software Co., Ltd. All Rights Reserved.\r\n\r\n");

    led_init(); 
    led_on();
    delay(100);
    int voltage = readBatteryVoltage();
    Serial.printf("Battery: %d mV\r\n", voltage);
    if(voltage < 2500) {
        Serial.println("[INFO]电池损坏或无ADC电路。");
    } else if(voltage < 3000) {
        Serial.println("[WARN]电量低于3v，系统休眠。");
        go_sleep();
    } else if (voltage < 3300) {
        // 低于3.3v，电池电量用尽，屏幕给警告，然后关机。 
        Serial.println("[WARN]电量低于3.3v，警告并系统休眠。");
        si_warning("电量不足，请充电！");
        go_sleep();
    } else if (voltage > 4400) {
        Serial.println("[INFO]未接电池。");
    }

    button.setClickMs(500); // 双击间隔时间，从深度睡眠唤醒后需要更大的窗口
    button.setPressMs(3000); // 设置长按的时长
    button.attachClick(buttonClick, &button);
    button.attachDoubleClick(buttonDoubleClick, &button);
    // button.attachMultiClick()
    button.attachLongPressStop(buttonLongPressStop, &button);
    attachInterrupt(digitalPinToInterrupt(KEY_M), checkTicks, CHANGE);

    Serial.println("Wm begin...");
    led_fast();
    wm.setHostname("J-Calendar");
    wm.setEnableConfigPortal(false);
    wm.setConnectTimeout(10);

    if (wm.autoConnect()) {
        Serial.println("Connect OK.");
        led_on();
        _wifi_flag = true;

        // 启动局域网配置服务
        lan_server.on("/", handleRoot);
        lan_server.on("/save", HTTP_POST, handleSave);
        lan_server.begin();
        Serial.printf("LAN Config: http://%s\n", WiFi.localIP().toString().c_str());
    } else {
        Serial.println("Connect failed.");
        _wifi_flag = false;
        _wifi_failed_millis = millis();

        // 检查是否是新设备（未配置过任何参数）
        Preferences pref;
        pref.begin(PREF_NAMESPACE);
        String _qweather_key = pref.getString(PREF_QWEATHER_KEY, "");
        pref.end();
        bool is_new_device = (_qweather_key.length() == 0);

        if (is_new_device) {
            // 新设备，显示配网提示页面
            Serial.println("New device detected, showing setup guide...");
            _new_device_mode = true;
            si_setup_guide("J-Calendar", "password");

            // 自动启动配置模式（AP 热点）
            Serial.println("Starting config portal for new device...");
            Serial.println("AP: J-Calendar, Password: password");
            led_config(); // LED 进入三快闪状态

            // 设置配置参数默认值
            Preferences pref;
            pref.begin(PREF_NAMESPACE);
            String qHost = pref.getString(PREF_QWEATHER_HOST, "api.qweather.com");
            String qType = pref.getString(PREF_QWEATHER_TYPE, "0");
            String week1st = pref.getString(PREF_SI_WEEK_1ST, "0");
            pref.end();

            para_qweather_host.setValue(qHost.c_str(), 64);
            para_qweather_type.setValue(qType.c_str(), 1);
            para_si_week_1st.setValue(week1st.c_str(), 1);

            // 添加配置参数
            wm.setTitle("J-Calendar");
            wm.addParameter(&para_si_week_1st);
            wm.addParameter(&para_qweather_host);
            wm.addParameter(&para_qweather_key);
            wm.addParameter(&para_qweather_type);
            wm.addParameter(&para_qweather_location);
            wm.addParameter(&para_cd_day_label);
            wm.addParameter(&para_cd_day_date);
            wm.addParameter(&para_tag_days);
            wm.addParameter(&para_study_schedule);

            // 设置配置菜单
            std::vector<const char*> menu = { "wifi","param","update","sep","info","restart","exit" };
            wm.setMenu(menu);
            wm.setConfigPortalBlocking(false);
            wm.setBreakAfterConfig(false);
            wm.setSaveParamsCallback(saveParamsCallback);
            wm.setSaveConnect(true);

            // 启动配置门户
            wm.startConfigPortal("J-Calendar", "password");

            // 不执行后续的 SNTP 和天气获取
            return;
        } else {
            // 老设备WiFi连接失败，显示警告
            Serial.println("WiFi connect failed for configured device.");
            si_warning("WiFi连接失败");
        }

        led_slow();
        _sntp_exec(2);
        weather_exec(2);
        WiFi.mode(WIFI_OFF); // 提前关闭WIFI，省电
        Serial.println("Wifi closed.");
    }
}

/**
 * 处理各个任务
 * 1. sntp同步
 *      前置条件：Wifi已连接
 * 2. 刷新日历
 *      前置条件：sntp同步完成（无论成功或失败）
 * 3. 刷新天气信息
 *      前置条件：wifi已连接
 * 4. 系统配置
 *      前置条件：无
 * 5. 休眠
 *      前置条件：所有任务都完成或失败，
 */
void loop() {
    button.tick(); // 单击，刷新页面；双击，打开配置；长按，重启

    // 新设备配网模式：处理配置门户事件
    if (_new_device_mode) {
        wm.process(); // 处理配置门户请求
        // 超时休眠（3分钟）
        if (millis() - _wifi_failed_millis > 180 * 1000) {
            Serial.println("New device mode timeout, going to sleep...");
            go_sleep();
        }
        delay(10);
        return;
    }

    wm.process();
    lan_server.handleClient();
    // 前置任务：wifi已连接
    // sntp同步
    if (_sntp_status() == -1) {
        _sntp_exec();
    }
    // 如果是定时器唤醒，并且接近午夜（23:50之后），则直接休眠
    if (_sntp_status() == SYNC_STATUS_TOO_LATE) {
        go_sleep();
    }
    // 前置任务：wifi已连接
    // 获取Weather信息
    if (weather_status() == -1) {
        weather_exec();
    }

    // 刷新日历
    // 前置任务：sntp、weather
    // 执行条件：屏幕状态为待处理
    if (_sntp_status() > 0 && weather_status() > 0 && si_screen_status() == -1) {
        Serial.println("Data fetch done, updating screen...");

        si_screen();
        _screen_done_millis = millis();
    }

    // 休眠
    // 前置条件：屏幕刷新完成（或成功）

    // 未在配置状态，且屏幕刷新完成，进入休眠
    if (!wm.getConfigPortalActive() && si_screen_status() > 0) {
        if (_wifi_flag) {
            // 检查天气是否已配置，未配置则保持更长唤醒时间
            Preferences pref;
            pref.begin(PREF_NAMESPACE);
            String _key = pref.getString(PREF_QWEATHER_KEY, "");
            pref.end();
            bool _weather_configured = _key.length() > 0;

            if (!_wakeup_by_button && _weather_configured) {
                go_sleep(); // 定时器唤醒且已配置，立即休眠
            }
            // 按键唤醒或天气未配置，等待一段时间再休眠
            unsigned long wait_time = _weather_configured ? 10 * 1000 : 60 * 1000;
            if (millis() - _screen_done_millis > wait_time) {
                go_sleep();
            }
        }
        if (!_wifi_flag && millis() - _wifi_failed_millis > 10 * 1000) { // 如果wifi连接不成功，等待10秒休眠
            go_sleep();
        }
    }
    // 配置状态下，
    if (wm.getConfigPortalActive() && millis() - _idle_millis > TIME_TO_SLEEP) {
        go_sleep();
    }

    delay(10);
}


// 刷新页面
void buttonClick(void* oneButton) {
    Serial.println("Button click.");
    if (wm.getConfigPortalActive()) {
        Serial.println("In config status.");
    } else {
        Serial.println("Refresh screen manually.");
        Preferences pref;
        pref.begin(PREF_NAMESPACE);
        int _si_type = pref.getInt(PREF_SI_TYPE);
        pref.putInt(PREF_SI_TYPE, _si_type == 0 ? 1 : 0);
        pref.end();
        si_screen();
    }
}

void saveParamsCallback() {
    Preferences pref;
    pref.begin(PREF_NAMESPACE);
    pref.putString(PREF_QWEATHER_HOST, para_qweather_host.getValue());
    pref.putString(PREF_QWEATHER_KEY, para_qweather_key.getValue());
    pref.putString(PREF_QWEATHER_TYPE, strcmp(para_qweather_type.getValue(), "1") == 0 ? "1" : "0");
    pref.putString(PREF_QWEATHER_LOC, para_qweather_location.getValue());
    pref.putString(PREF_CD_DAY_LABLE, para_cd_day_label.getValue());
    pref.putString(PREF_CD_DAY_DATE, para_cd_day_date.getValue());
    pref.putString(PREF_TAG_DAYS, para_tag_days.getValue());
    pref.putString(PREF_SI_WEEK_1ST, strcmp(para_si_week_1st.getValue(), "1") == 0 ? "1" : "0");
    pref.putString(PREF_STUDY_SCHEDULE, para_study_schedule.getValue());
    pref.end();

    Serial.println("Params saved.");

    _idle_millis = millis(); // 刷新无操作时间点

    ESP.restart();
}

void preSaveParamsCallback() {
}

// 双击打开配置页面
void buttonDoubleClick(void* oneButton) {
    Serial.println("Button double click.");
    if (wm.getConfigPortalActive()) {
        ESP.restart();
        return;
    }

    if (weather_status() == 0) {
        weather_stop();
    }

    // 设置配置页面
    // 根据配置信息设置默认值
    Preferences pref;
    pref.begin(PREF_NAMESPACE);
    String qHost = pref.getString(PREF_QWEATHER_HOST);
    String qToken = pref.getString(PREF_QWEATHER_KEY);
    String qType = pref.getString(PREF_QWEATHER_TYPE, "0");
    String qLoc = pref.getString(PREF_QWEATHER_LOC);
    String cddLabel = pref.getString(PREF_CD_DAY_LABLE);
    String cddDate = pref.getString(PREF_CD_DAY_DATE);
    String tagDays = pref.getString(PREF_TAG_DAYS);
    String week1st = pref.getString(PREF_SI_WEEK_1ST, "0");
    String studySchedule = pref.getString(PREF_STUDY_SCHEDULE);
    pref.end();

    para_qweather_host.setValue(qHost.c_str(), 64);
    para_qweather_key.setValue(qToken.c_str(), 32);
    para_qweather_location.setValue(qLoc.c_str(), 64);
    para_qweather_type.setValue(qType.c_str(), 1);
    para_cd_day_label.setValue(cddLabel.c_str(), 16);
    para_cd_day_date.setValue(cddDate.c_str(), 8);
    para_tag_days.setValue(tagDays.c_str(), 30);
    para_si_week_1st.setValue(week1st.c_str(), 1);
    para_study_schedule.setValue(studySchedule.c_str(), 4000);

    wm.setTitle("J-Calendar");
    wm.addParameter(&para_si_week_1st);
    wm.addParameter(&para_qweather_host);
    wm.addParameter(&para_qweather_key);
    wm.addParameter(&para_qweather_type);
    wm.addParameter(&para_qweather_location);
    wm.addParameter(&para_cd_day_label);
    wm.addParameter(&para_cd_day_date);
    wm.addParameter(&para_tag_days);
    wm.addParameter(&para_study_schedule);
    // std::vector<const char *> menu = {"wifi","wifinoscan","info","param","custom","close","sep","erase","update","restart","exit"};
    std::vector<const char*> menu = { "wifi","param","update","sep","info","restart","exit" };
    wm.setMenu(menu); // custom menu, pass vector
    wm.setConfigPortalBlocking(true); // 阻塞模式，确保配置页面正常运行
    wm.setBreakAfterConfig(false); // 保存wifi后不退出配置页面，以便继续配置param
    wm.setPreSaveParamsCallback(preSaveParamsCallback);
    wm.setSaveParamsCallback(saveParamsCallback);
    wm.setSaveConnect(true); // 保存wifi后尝试连接，显示连接状态给用户反馈

    Serial.println("Starting config portal...");
    Serial.println("AP: J-Calendar, Password: password");
    Serial.println("Please connect to AP and visit: http://192.168.4.1");

    led_config(); // LED 进入三快闪状态

    // 控制配置超时180秒后休眠
    _idle_millis = millis();
    wm.startConfigPortal("J-Calendar", "password");

    // 配置页面退出后（超时或用户关闭），重启设备
    Serial.println("Config portal closed, restarting...");
    ESP.restart();
}

// 重置系统，并重启
void buttonLongPressStop(void* oneButton) {
    Serial.println("Button long press.");

    // 删除Preferences，namespace下所有健值对。
    Preferences pref;
    pref.begin(PREF_NAMESPACE);
    pref.clear();
    pref.end();

    ESP.restart();
}

#define uS_TO_S_FACTOR 1000000
#define TIMEOUT_TO_SLEEP  10 // seconds
time_t blankTime = 0;
void go_sleep() {
    uint64_t p;

    // 关闭WiFi，省电
    WiFi.mode(WIFI_OFF);

    // 根据配置情况来刷新，如果未配置qweather信息，则24小时刷新，否则每2小时刷新
    Preferences pref;
    pref.begin(PREF_NAMESPACE);
    String _qweather_key = pref.getString(PREF_QWEATHER_KEY, "");
    pref.end();

    time_t now;
    time(&now);
    struct tm local;
    localtime_r(&now, &local);
    if (_qweather_key.length() == 0 || weather_type() == 0) { // 没有配置天气或者使用按日天气，则第二天刷新。
        // Sleep to next day
        int secondsToNextDay = (24 - local.tm_hour) * 3600 - local.tm_min * 60 - local.tm_sec;
        Serial.printf("Seconds to next day: %d seconds.\n", secondsToNextDay);
        p = (uint64_t)(secondsToNextDay);
        p = p < 0 ? 3600 * 24 : (p + 30); // 额外增加30秒，避免过早唤醒
    } else {
        // Sleep to next even hour.
        int secondsToNextHour = (60 - local.tm_min) * 60 - local.tm_sec;
        if ((local.tm_hour % 2) == 0) { // 如果是奇数点，则多睡1小时
            secondsToNextHour += 3600;
        }
        Serial.printf("Seconds to next even hour: %d seconds.\n", secondsToNextHour);
        p = (uint64_t)(secondsToNextHour);
        p = p < 0 ? 3600 : (p + 10); // 额外增加10秒，避免过早唤醒
    }

    esp_sleep_enable_timer_wakeup(p * (uint64_t)uS_TO_S_FACTOR);
    esp_sleep_enable_ext0_wakeup(KEY_M, LOW);

    // 省电考虑，关闭RTC外设和存储器
    // esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF); // RTC IO, sensors and ULP, 注意：由于需要按键唤醒，所以不能关闭，否则会导致RTC_IO唤醒(ext0)失败
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF); // 
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC8M, ESP_PD_OPTION_OFF);

    gpio_deep_sleep_hold_dis(); // 解除所有引脚的保持状态
    
    // 省电考虑，重置gpio，平均每针脚能省8ua。
    // gpio_reset_pin(PIN_LED_R); // 减小deep-sleep电流
    gpio_reset_pin(SPI_CS); // 减小deep-sleep电流
    gpio_reset_pin(SPI_DC); // 减小deep-sleep电流
    gpio_reset_pin(SPI_RST); // 减小deep-sleep电流
    gpio_reset_pin(SPI_BUSY); // 减小deep-sleep电流`
    gpio_reset_pin(SPI_MOSI); // 减小deep-sleep电流
    gpio_reset_pin(SPI_MISO); // 减小deep-sleep电流
    gpio_reset_pin(SPI_SCK); // 减小deep-sleep电流
    // gpio_reset_pin(PIN_ADC); // 减小deep-sleep电流
    // gpio_reset_pin(I2C_SDA); // 减小deep-sleep电流
    // gpio_reset_pin(I2C_SCL); // 减小deep-sleep电流

    delay(10);
    Serial.println("Deep sleep...");
    Serial.flush();
    esp_deep_sleep_start();
}