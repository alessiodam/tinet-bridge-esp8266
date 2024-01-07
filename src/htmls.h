const char SETUP_ROOT_PAGE_HTML[] PROGMEM = R"=====(
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
  <h2>TINET Bridge WiFi Setup</h2>
  <form action='/saveconfig' method='post'>
    <label><img class="wifi-icon" src="wifi-icon.png" alt="WiFi"> SSID (max. 32 chars): </label>
    <input type='text' name='ssid' /><br>
    <label><img class="wifi-icon" src="password-icon.png" alt="Password"> Password (max. 64 chars): </label>
    <input type='password' name='password' /><br>
    <input type='submit' value='Set' /></form><br><br><br>
    <form action='/reset' method='post'>
    <input type='submit' value='Reset to Factory Settings' onclick='return confirm("Are you sure? You will need to set up WiFi again!");' />
  </form>
</body>

</html>
)=====";

const char SETUP_SAVE_CONFIG_HTML[] PROGMEM = R"=====(
WiFi set up success, your bridge will reboot and connect to wifi<b>
If WiFi connect failed after 10 seconds, your bridge will boot up again in setup mode and you will need to re-do the setup steps
)=====";

const char RESET_HTML[] PROGMEM = R"=====(
Reset to factory settings successful. <a href='/'>Go to Management Page</a>
)=====";

const char ROOT_HTML[] PROGMEM = R"=====(
<html>

<head>
  <title>TINET bridge admin page</title>
</head>

<body>
  <h2>Management Page</h2>
  <h3>more will come here later</h3><br>
  <form action='/setpassword' method='post'>
    <label>Password (please do this on your local network for better security! Max 64 chars.): </label>
    <input type='password' name='password'/>
    <input type='submit' value='Set'/></form>
    <form action='/reset' method='post'>
    <input type='submit' value='Reset to Factory Settings' onclick='return confirm(\"Are you sure?\");'/>
  </form>
</body>

</html>
)=====";

const char ROOT_NO_PASSWORD_HTML[] PROGMEM = R"=====(
Please set a password <a href='/setpassword'>here</a>.
)=====";

const char SET_PASSWORD_HTML[] PROGMEM = R"=====(
<html>

<body>
    <form action='/savepassword' method='post'>
    <label>Password: </label>
    <input type='password' name='password'/>
    <input type='submit' value='Set'/>
  </form>
</body>

</html>
)=====";

const char SET_PASSWORD_SUCCESS_HTML[] PROGMEM = R"=====(
Password set successfully. <a href='/'>Go to Management Page</a>
)=====";

const char UPDATE_FAILED_HTML[] PROGMEM = R"=====(
Update failed.
)=====";

const char UPDATE_SUCCESS_HTML[] PROGMEM = R"=====(
Update success! Bridge will reboot now!
)=====";

const char NO_UPDATES_AVAILABLE_HTML[] PROGMEM = R"=====(
No updates available!
)=====";
