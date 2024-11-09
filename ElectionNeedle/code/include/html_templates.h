#ifndef HTML_TEMPLATES_H
#define HTML_TEMPLATES_H

const char* CONFIG_HTML = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Manifold Servo Configuration</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial; margin: 0; padding: 20px; }
        .container { max-width: 400px; margin: 0 auto; }
        input, button { width: 100%; padding: 10px; margin: 10px 0; box-sizing: border-box; }
        button { background: #4CAF50; color: white; border: none; cursor: pointer; }
        button:hover { background: #45a049; }
        .status { margin-top: 20px; }
    </style>
</head>
<body>
    <div class="container">
        <h1>Manifold Servo Setup</h1>
        <form id="wifiForm">
            <input type="text" id="ssid" name="ssid" placeholder="WiFi SSID" required>
            <input type="password" id="password" name="password" placeholder="WiFi Password" required>
            <button type="submit">Connect</button>
        </form>
        <div id="status" class="status"></div>
    </div>
    <script>
        document.getElementById("wifiForm").onsubmit = function(e) {
            e.preventDefault();
            var status = document.getElementById("status");
            status.textContent = "Connecting...";

            fetch("/configure", {
                method: "POST",
                headers: {"Content-Type": "application/x-www-form-urlencoded"},
                body: new URLSearchParams(new FormData(e.target))
            })
            .then(response => response.json())
            .then(data => {
                if (data.success) {
                    status.textContent = "Connected! Device will restart. You can access it at: http://" +
                        data.hostname + " or http://" + data.ip;
                } else {
                    status.textContent = data.message || "Connection failed";
                }
            })
            .catch(err => {
                status.textContent = "Error: " + err.message;
            });
        };
    </script>
</body>
</html>
)rawliteral";

const char* MAIN_HTML = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Manifold Servo Control</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial; margin: 0; padding: 20px; }
        .container { max-width: 600px; margin: 0 auto; }
        .card {
            background: #f5f5f5;
            border-radius: 8px;
            padding: 20px;
            margin: 20px 0;
        }
        input, button { width: 100%; padding: 10px; margin: 10px 0; box-sizing: border-box; }
        button { background: #4CAF50; color: white; border: none; cursor: pointer; }
        button:hover { background: #45a049; }
        .status { margin-top: 20px; }
        #updateStatus { color: #4CAF50; }
        #errorStatus { color: #f44336; }
    </style>
</head>
<body>
    <div class="container">
        <h1>Manifold Servo Control</h1>

        <div class="card">
            <h2>Current Status</h2>
            <p>Market: <strong id="currentSlug">%CURRENT_SLUG%</strong></p>
            <p>Probability: <strong id="currentProb">%CURRENT_PROB%</strong></p>
            <p>Servo Angle: <strong id="currentAngle">%CURRENT_ANGLE%</strong>°</p>
            <button onclick="doRefresh()">Refresh Status</button>
        </div>

        <div class="card">
            <h2>Configure Market</h2>
            <form id="slugForm">
                <input type="text" id="slug" name="slug" placeholder="Market Slug"
                       value="%CURRENT_SLUG%" required>
                <small style="display: block; margin: 5px 0;">
                    Enter the slug from the market URL (e.g., "will-trump-win-the-2024-election")
                </small>
                <button type="submit">Update Market</button>
            </form>
            <p id="updateStatus"></p>
            <p id="errorStatus"></p>
        </div>
    </div>

    <script>
        function doRefresh() {
            fetch("/status")
                .then(response => response.json())
                .then(data => {
                    document.getElementById("currentSlug").textContent = data.slug;
                    document.getElementById("currentProb").textContent =
                        (data.probability * 100).toFixed(1) + "%";
                    document.getElementById("currentAngle").textContent = data.angle;
                })
                .catch(err => {
                    document.getElementById("errorStatus").textContent =
                        "Error refreshing status: " + err.message;
                });
        }

        document.getElementById("slugForm").onsubmit = function(e) {
            e.preventDefault();
            var updateStatus = document.getElementById("updateStatus");
            var errorStatus = document.getElementById("errorStatus");
            updateStatus.textContent = "Updating...";
            errorStatus.textContent = "";

            fetch("/update-slug", {
                method: "POST",
                headers: {"Content-Type": "application/x-www-form-urlencoded"},
                body: new URLSearchParams(new FormData(e.target))
            })
            .then(response => response.json())
            .then(data => {
                if (data.success) {
                    updateStatus.textContent = "Market updated successfully!";
                    setTimeout(doRefresh, 1000);
                } else {
                    errorStatus.textContent = data.message || "Update failed";
                }
            })
            .catch(err => {
                errorStatus.textContent = "Error: " + err.message;
            });
        };

        // Refresh status every 5 seconds
        setInterval(doRefresh, 5000);
    </script>
</body>
</html>
)rawliteral";

#endif // HTML_TEMPLATES_H
