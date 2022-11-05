# esp8266_evn_smartmeter

## Einführung

Dieses Projekt implementiert das Auslesen eines NETZ-NÖ-Smartmeters mittels eines ESP8266 sowie einem Pegelwandler von MBUS auf UART.

Folgende Informationen stellt die NETZ NÖ bereit:
https://www.netz-noe.at/Download-(1)/Smart-Meter/218_9_SmartMeter_Kundenschnittstelle_lektoriert_14.aspx

Diese sind nur bedingt aussagekräftig, da dort einige Probleme nicht beschrieben werden und die dort erwähnten Tools nicht auf Microcontrollern laufen.

## Hardware

Ich habe mich für eine Implementierung auf einem ESP8266 entschieden, da dieser relativ wenig Strom verbraucht und ich ursprünglich die Stromversorgung auch direkt aus dem M-Bus ermöglichen wollte.
Leider geben die Smart-Meter nicht genügend Strom (= MBUS Unit Loads) her, sodass eine externe Stromversorgung per Hutschienennetzteil notwendig ist.

Der von mir eingesetzte Pegelwandler ist von Texas Instruments, ein TSSS721A (https://www.ti.com/lit/ds/symlink/tss721a.pdf?ts=1667630300522&ref_url=https%253A%252F%252Fwww.google.com%252F)
Diesen gibt es auch als fertiges Board relativ kostengünstig von MIKROE: https://www.mikroe.com/m-bus-slave-click

Dieser setzt MBUS auf UART um. 
Die Dokumentation von MBUS ist im Internet vorhanden aber nicht besonders übersichtlich.
Relevant sind vor allem die Baudrate (2400) und die Parität (8E1). Da aber der ESP8266 nur eine Hardware-UART-Schnittstelle besitzt habe ich den Anschluss über SoftwareSerial realisiert, was eigentlich nur 8N1 kann. Es funktioniert trotzdem.

## Smart-Meter

Getestet ist das ganze mit einem Sagemcom Drehstromzähler T210-D. Man muss im Vorhinein den Entschlüsselungscode bei der EVN beantragen.
Dieser kommt per Email als verschlüsseltes ZIP, dessen Schlüssel man per SMS bekommt.
Diesen muss man anschließend in den Code für den ESP8266 integrieren.

## Compiler

Geschrieben wurde das ganze in der Arduino-Umgebung mit einem ESP8266 als Target. Es sollten alle Arten von ESP8266 bzw. vermutlich auch ESP32 funktionieren.
Vorsicht, wenn man den Code auf eine Architektur kopieren will, da die Byteoperationen auf AVR als Architektur ausgelegt sind. 

## MQTT

Die Daten werden anschließend per WLAN zu einem MQTT-Broker transferiert. Es sind aber auch andere Alternativen denkbar. 

## Verfügbare Daten

Folgende Daten sind verfügbar [Einheiten in eckigen Klammern]:
- Aktuelle Wirkenergie Plus/Minus [kWH]
- Momentanleistung Plus/Minus [W]
- Momemtanstrom aller drei Phasen (leider ohne Vorzeichen) [A]
- Spannung aller drei Phasen [V]
- Leistungsfaktor (funktioniert aber in dem Code noch nicht richtig) []
