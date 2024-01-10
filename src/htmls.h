const char SETUP_ROOT_PAGE_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">

<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>TINET Bridge Setup</title>

  <style>
    body {
      font-family: 'Arial', sans-serif;
      background-color: #f4f4f4;
      margin: 0;
      padding: 0;
    }

    h2 {
      color: #333;
    }

    form {
      max-width: 400px;
      margin: 20px auto;
      padding: 20px;
      background-color: #fff;
      box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
    }

    label {
      display: block;
      margin-bottom: 8px;
      color: #555;
    }

    input {
      width: 100%;
      padding: 8px;
      margin-bottom: 16px;
      box-sizing: border-box;
    }

    input[type="submit"] {
      background-color: #4caf50;
      color: #fff;
      cursor: pointer;
    }

    input[type="submit"]:hover {
      background-color: #45a049;
    }

    input[type="password"],
    input[type="text"] {
      border: 1px solid #ccc;
      border-radius: 4px;
    }

    .wifi-icon {
      width: 24px;
      height: 24px;
      margin-right: 8px;
    }
  </style>
</head>

<body>
  <h2>TINET Bridge Setup</h2>
  <form action='/saveconfig' method='post'>
    <label>WiFi SSID (max. 32 chars): </label>
    <input type='text' name='wifi_ssid' /><br>
    <label>WiFi Password (max. 64 chars): </label>
    <input type='password' name='wifi_password' /><br>
    <label>Bridge Admin Password (max. 64 chars): </label>
    <input type='password' name='password' /><br>
    <input type='submit' value='Set' /></form><br><br><br>
    <form action='/reset' method='post'>
    <input type='submit' value='Reset to Factory Settings' onclick='return confirm("Are you sure? You will need to set up WiFi again!");' />
  </form>
</body>

</html>
)rawliteral";

const char SETUP_SAVE_CONFIG_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        body {
            font-family: Arial, sans-serif;
            background-color: #e6f7ff;
            text-align: center;
            margin: 50px;
        }

        b {
            color: #4CAF50;
        }

        p {
            color: #333;
        }
    </style>
    <title>WiFi Setup Status</title>
</head>
<body>
    <p><b>WiFi set up success,</b> your bridge will reboot and connect to WiFi.</p>
    <p>If WiFi connection fails after 10 seconds, your bridge will boot up again in setup mode, and you will need to re-do the setup steps.</p>
</body>
</html>
)rawliteral";

const char RESET_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        body {
            font-family: Arial, sans-serif;
            background-color: #e6ffe6;
            text-align: center;
            margin: 50px;
        }

        a {
            text-decoration: none;
            color: #3498db;
            font-weight: bold;
        }
    </style>
    <title>Factory Reset Successful</title>
</head>
<body>
    <p><b>Reset to factory settings successful.</b> <a href='/'>Go to Management Page</a></p>
</body>
</html>
)rawliteral";

const char ROOT_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">

<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>TINET Bridge</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      text-align: center;
      margin: 20px;
      background-color: #f4f4f4;
    }

    h2 {
      color: #333;
    }

    label {
      display: block;
      margin-top: 10px;
    }

    input {
      margin-bottom: 10px;
      padding: 8px;
      border: 1px solid #ccc;
      border-radius: 4px;
    }

    input[type='submit'] {
      background-color: #4caf50;
      color: white;
      padding: 10px 15px;
      border: none;
      border-radius: 4px;
      cursor: pointer;
    }

    input[type='submit']:hover {
      background-color: #45a049;
    }
  </style>
</head>

<body>
  <h2>TINET Bridge Dashboard</h2>
  <form action='/setpassword' method='get'>
    <input type='submit' value='Set a new password' />
  </form>

  <form action='/update' method='post'>
    <input type='submit' value='Update' />
  </form>

  <form action='/reset' method='post'>
    <input type='submit' value='Reset to Factory Settings' />
  </form>
</body>

</html>
)rawliteral";

const char SET_PASSWORD_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Password Setup</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            text-align: center;
            margin: 20px;
            background-color: #f4f4f4;
        }

        label {
            display: block;
            margin-top: 10px;
        }

        input {
            padding: 8px;
            border: 1px solid #ccc;
            border-radius: 4px;
            margin-bottom: 10px;
        }

        input[type='submit'] {
            background-color: #4caf50;
            color: white;
            padding: 10px 15px;
            border: none;
            border-radius: 4px;
            cursor: pointer;
        }

        input[type='submit']:hover {
            background-color: #45a049;
        }
    </style>
</head>

<body>
    <form action='/savepassword' method='post'>
        <label for='password'>Password:</label>
        <input type='password' name='password' id='password' required/>
        <br>
        <input type='submit' value='Set Password'/>
    </form>
</body>

</html>
)rawliteral";

const char SET_PASSWORD_SUCCESS_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Password Set Success</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            text-align: center;
            margin: 50px;
            background-color: #e6ffe6;
        }

        a {
            text-decoration: none;
            color: #3498db;
            font-weight: bold;
        }
    </style>
</head>

<body>
    <p>Password set successfully. <a href='/'>Go to Management Page</a></p>
</body>

</html>
)rawliteral";

const char UPDATE_FAILED_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Update Failed</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            text-align: center;
            margin: 50px;
            background-color: #ffcccc;
        }

        p {
            color: #333;
        }
    </style>
</head>

<body>
    <p><b>Update failed.</b></p>
</body>

</html>
)rawliteral";

const char UPDATE_SUCCESS_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Update Success</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            text-align: center;
            margin: 50px;
            background-color: #ccffcc;
        }

        p {
            color: #333;
        }
    </style>
</head>

<body>
    <p><b>Update success!</b> Bridge will reboot now!</p>
</body>

</html>
)rawliteral";

const char NO_UPDATES_AVAILABLE_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>No Updates Available</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            text-align: center;
            margin: 50px;
            background-color: #e6f7ff;
        }

        p {
            color: #333;
        }
    </style>
</head>

<body>
    <p><b>No updates available!</b></p>
</body>

</html>
)rawliteral";

const char LOGGED_OUT_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>

<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
</head>

<body>
  <p>Logged out! <a href="/">return to homepage</a>.</p>
  <p><strong>Note:</strong> close all web browser tabs to complete the logout process.</p>
</body>

</html>
)rawliteral";
