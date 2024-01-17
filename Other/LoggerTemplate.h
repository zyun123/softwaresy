#pragma once
#include <QString>
#include <string>
namespace Logger {
const static auto logTemplate = QString::fromUtf8(u8R"logTemplate(
<?xml version="1.0" encoding="utf-8" standalone="yes"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>

<head>
    <title>sy3 log file</title>
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
    <style type="text/css" id="logCss">
        body {
            background: #18242b;
            color: #afc6d1;
            margin-right: 20px;
            margin-left: 20px;
            font-size: 14px;
            font-family: Arial, sans-serif, sans;
        }

        a {
            text-decoration: none;
        }

        a:link {
            color: #a0b2bb;
        }

        a:active {
            color: #f59504;
        }

        a:visited {
            color: #adc7d4;
        }

        a:hover {
            color: #e49115;
        }

        h1 {
            text-align: center;
        }

        h2 {
            color: #ebe5e5;
        }

        .info,
        .exception-sys,
        .exception-ai,
        .exception-bot,
        .exception-cam,
        .exception-bed,
        .robotl,
        .robotr {
            padding: 3px;
            overflow: auto;
        }

        .info,
        .robotl,
        .robotr {
            background-color: #0f1011;
            color: #a8c1ce;
        }

        .exception-sys,
        .exception-ai,
        .exception-bot,
        .exception-cam,
        .exception-bed {
            background-color: #0f1011;
            color: #ff0000;
        }
    </style>
</head>

<body>
    <h1>SY3日志文件</h1>
    <script type="text/JavaScript">
        function objHide(obj) {
            obj.style.display="none"
        }
        function objShow(obj) {
            obj.style.display="block"
        }
        function selectType() {
            var all_list = new Set(['info', 'robotl', 'robotr', 'exception-sys', 'exception-ai', 'exception-bot', 'exception-cam', 'exception-bed']);
            var show_list = new Set(['info', 'robotl', 'robotr', 'exception-sys', 'exception-ai', 'exception-bot', 'exception-cam', 'exception-bed']);
            var check_info = document.getElementById('Info').checked;
            var check_esys = document.getElementById('Exception-sys').checked;
            var check_eai = document.getElementById('Exception-AI').checked;
            var check_ebot = document.getElementById('Exception-bot').checked;
            var check_ecam = document.getElementById('Exception-cam').checked;
            var check_ebed = document.getElementById('Exception-bed').checked;
            var check_mbotl = document.getElementById('RobotLeftMessage').checked;
            var check_mbotr = document.getElementById('RobotRightMessage').checked;;
            if(check_info === false)
                show_list.delete('info')
            if(check_esys === false)
                show_list.delete('exception-sys')
            if(check_eai === false)
                show_list.delete('exception-ai')
            if(check_ebot === false)
                show_list.delete('exception-bot')
            if(check_ecam === false)
                show_list.delete('exception-cam')
            if(check_ebed === false)
                show_list.delete('exception-bed')
            if(check_mbotl === false)
                show_list.delete('robotl')
            if(check_mbotr === false)
                show_list.delete('robotr')
            all_list.forEach(element => {
                var list = document.querySelectorAll('.' + element);
                list.forEach(objHide);
            });
            show_list.forEach(element => {
                var list = document.querySelectorAll('.' + element);
                list.forEach(objShow);
            });
        }
    </script>
    <div>
        <input type="checkbox" id="Info" name="Info" checked onchange="selectType()">
        <label for="Info">普通</label>
        <input type="checkbox" id="Exception-sys" name="Exception-sys" checked onchange="selectType()">
        <label for="Exception-sys">异常-系统</label>
        <input type="checkbox" id="Exception-AI" name="Exception-AI" checked onchange="selectType()">
        <label for="Exception-AI">异常-AI</label>
        <input type="checkbox" id="Exception-bot" name="Exception-bot" checked onchange="selectType()">
        <label for="Exception-bot">异常-机械臂</label>
        <input type="checkbox" id="Exception-cam" name="Exception-cam" checked onchange="selectType()">
        <label for="Exception-cam">异常-相机</label>
        <input type="checkbox" id="Exception-bed" name="Exception-bed" checked onchange="selectType()">
        <label for="Exception-bed">异常-床</label>
        <input type="checkbox" id="RobotLeftMessage" name="RobotLeftMessage" checked onchange="selectType()">
        <label for="RobotLeftMessage">左机械臂通信</label>
        <input type="checkbox" id="RobotRightMessage" name="RobotRightMessage" checked onchange="selectType()">
        <label for="RobotRightMessage">右机械臂通信</label>
    </div>
)logTemplate");
}
