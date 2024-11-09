# Manifold Market Election Needle

A physical indicator that tracks Manifold Market predictions in real-time.

![IMG_3500](https://github.com/user-attachments/assets/70cc6794-e316-479c-8a40-bb3effc462c4)
![electionneedle](https://github.com/user-attachments/assets/59814814-2350-480e-9bff-4d99daf540a0)

## Components Required
1. ESP32
2. SG90 servo
3. 3 male to female dupont connectors
4. A semi circle of your choosing (I laser cut my big ones out of poster foam board)
5. A kebab stick
6. Super glue
7. Data micro USB cable

## Instructions

### Note
Use your data micro USB cable to connect your ESP32 to your laptop.

### Setup Steps
1. Install PlatformIO
2. Go to the code directory and run `pio run -t upload` to add the code to your microcontroller
3. Hook up servo ground, power, and analog to the VIN GND and D13 pins using your dupont cables. You can use a different analog pin but then you will have to change code
4. Super glue kebab stick to the servo, you might have to clip it together while it dries
5. Attach servo to semi circle
6. Using your phone or computer, go to where you would go to connect to wifi, you should see ElectionNeedleConfig as an option
7. Connect to the captive portal, enter your wifi credentials. Give it a minute or two
8. You should now be able to type electionneedle.local into any browser and access this election needle, it will let you change the slug

Congrats! You can now get real life updates from any market hosted by manifold.
