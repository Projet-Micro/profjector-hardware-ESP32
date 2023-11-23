# ESP32 Hardware
This repository exposes the main features making the mobile & the `ESP32` communicate through the Bluetooth network
# Resources needed
- The `ESP32` Micro-controller
- ArduinoIDE 2.2.1 ( The HTTPClient and ArduinoJson have to be installed from the Arduino Library)
# Features
- The ESP's role is to intercept the professors requests that are in the **radius of the Bluetooth Network**. So, all the requests must be done
in the university. That way, we'll be sure that someone can only borrow a projector if he's in the university.
- The ESP will intercept the request and call an API in order to save the borrow request done by the professor into the database.
- The ESP intercepts the data transmitted by the mobile app ( in a string format ). Then the micro-controller converts it to a JSON to make the `POST` request to the `Historique` table.