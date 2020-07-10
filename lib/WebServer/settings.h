const char SETTINGS_HTML[] = R"=====(
<html lang="de">
    <head>
        <link rel="stylesheet" href="style.css">
        <link rel="icon" type="image/svg" href="logo.svg">
        <meta charset="utf-8">
        <title>Zeit Messung</title>
    </head>
    <body>
        <header>
            <nav>
                <div id="navCenter">
                    <a touchstart="this.classList.add('navHover');" touchend="this.classList.remove('navHover');" onmouseover="this.classList.add('navHover');" onmouseout="this.classList.remove('navHover');" class="navLink" href="/">View</a>
                    <a touchstart="this.classList.add('navHover');" touchend="this.classList.remove('navHove');" onmouseover="this.classList.add('navHover');" onmouseout="this.classList.remove('navHover');" class="navLink" href="/wifiroot">WiFi</a>
                    <a touchstart="this.classList.add('navHover');" touchend="this.classList.remove('navHover');" onmouseover="this.classList.add('navHover');" onmouseout="this.classList.remove('navHover');" class="navLink" href="#">Settings</a>
                </div>
            </nav>
        </header>
        <main>
            <label for="minCooldown">Min Cooldown(ms): </label><input type="number" id="minCooldown" onchange="minCooldownChanged();">
            <hr>
        </main>
        <script>
            "useStrict";
            const xhr = new XMLHttpRequest();

            function minCooldownChanged(){
                xhr.open("GET", "/settings?minCooldown=" + document.getElementById("minCooldown").value, true);
                xhr.send();
                xhr.close();
            }
        </script>
    </body>
</html>
)=====";