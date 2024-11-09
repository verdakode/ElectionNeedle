MANIFOLD MARKET ELECTION NEEDLE

![IMG_3500](https://github.com/user-attachments/assets/70cc6794-e316-479c-8a40-bb3effc462c4)


Ingredients:

1. Esp32
2. sg90 servo
3. 3 male to female dupont connectors
4. a semi circle of your choosing ( I laser cut my big ones out of poster foam board )
5. a kebab stick
6. super glue
7. data micro usb cable

Instructions:

Note
Use your data micro usb cable to connect your esp32 to your laptop.
Install platformio.
Go to the code directory and run pio run -t upload to add the code to your microcontroller.
Hook up servo ground, power, and analog to the VIN GND and D13 pins using your dupont cables. You can use a different analog pin but then you will have to change code.
At some point, super glue kebab stick to the servo, you might have to clip it together while it dries.
Attach servo to semi circle
Using your phone or computer, go to where you would go to connect to wifi, you should see ElectionNeedleConfig as an option.
Connect to the captive portal, enter your wifi credentials. Give it a minute or two.
You should now be able to type electionneedle.local into any browser and access this election needle, it will let you change the slug
Congrats! You can now get real life updates from any market hosted by manifold.
