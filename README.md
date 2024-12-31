# LIN bus demo using VW Golf VII / Audi A3 8V wiper actuator (STM32G474RE)
My strong inclination towards electric drives made me knock together a sequel to the [Toyota RAV4 climate control panel](https://github.com/ufnalski/lin_bus_rav4_climate_control_g474re). We are going to communicate over the LIN bus with a wiper actuator. There is almost endless selection of such actuators on the market. However, my goal here is not to spend time on hacking one. The exercise is supposed to be a datasheet-based one. Searching for _wiper actuator/motor protocol_ gave me Wiper Direct Actuator (WDA) from Bosch Motorsport. A quite pricey item. Let's experiment with something cheaper - a similarly looking actuator being used in VW Golf VII, Audi A3 8V, and probably many other popular models. The used part can be easily bought for under $10.

![LIN bus wiper actuator in action](/Assets/Images/golf7_wda_in_action.jpg)

![WDA purchasing options](/Assets/Images/golf7_wda_qr_code.JPG)

# Missing files?
Don't worry :slightly_smiling_face: Just hit Alt-K to generate /Drivers/CMCIS/ and /Drivers/STM32G4xx_HAL_Driver/ based on the .ioc file. After a couple of seconds your project will be ready for building.

# Libraries
* [stm32-ssd1306](https://github.com/afiskon/stm32-ssd1306) (MIT license)
* [QR Code generator library](https://www.nayuki.io/page/qr-code-generator-library)[^1] (MIT license)

[^1]: The QR code will take you to the wiper direct actuator (WDA) purchasing options.

# Exemplary LIN transceivers
* [Dual LIN click](https://www.mikroe.com/dual-lin-click) (MIKROE)
* [LIN Click](https://www.mikroe.com/lin-click) (MIKROE)
* [MCP2003B Click](https://www.mikroe.com/mcp2003b-click) (MIKROE)

# Readings
* [Local Interconnect Network](https://en.wikipedia.org/wiki/Local_Interconnect_Network) (Wikipedia)
* [LIN Bus Explained - A Simple Intro [2023]](https://www.csselectronics.com/pages/lin-bus-protocol-intro-basics) (CSS Electronics)
* [Introduction to the LIN bus](https://kvaser.com/about-can/can-standards/linbus/) (KVASER)
* [STM32 as the Lin Master || Configure & Send Data](https://controllerstech.com/stm32-uart-8-lin-protocol-part-1/) (ControllersTech)
* [Using Lin Transceivers to communicate between master and slave](https://controllerstech.com/stm32-uart-9-lin-protocol-part-2/) (ControllersTech)
* [Master - slave communication using LinBus](https://controllerstech.com/stm32-uart-10-lin-protocol-part-3/) (ControllersTech)
* [Wiper Direct Actuator WDA (datasheet with protocol description)](https://www.bosch-motorsport.com/content/downloads/Raceparts/Resources/pdf/Data%20Sheet_68715659_Wiper_Direct_Actuator_WDA.pdf) (Bosch Motorsport)
* [LIN Specification Package Revision 2.2A](https://www.cs-group.de/wp-content/uploads/2016/11/LIN_Specification_Package_2.2A.pdf)
* [VW Golf 7 sterowanie silnikiem wycieraczek przod siec LIN. BUS LIN.](https://www.youtube.com/watch?v=aTWoo6geNao) (eCarEdu.pl)

> [!IMPORTANT]
> The WDA for motorsport has visibly different casing but it uses an almost identical communication protocol. "Almost" makes a difference between being able to communicate and not being able to communicate at all. It took me a lot of guessing and checking to figure out that the "civilian" version has a longer data field. They've probably shortened the frame for motorsport to make it faster :wink: Follow the datasheet and then just add two zero bytes to the payload.

# Exemplary sniffers/decoders
* [DSLogic Plus](https://www.dreamsourcelab.com/product/dslogic-series/) (DreamSourceLab)
* [PLIN-USB: LIN Interface for USB](https://www.peak-system.com/PLIN-USB.485.0.html?&L=1) (PEAK-System)

![LIN bus decoder](/Assets/Images/dsview_linbus_golf7_wda.JPG)
![LIN bus interface/sniffer](/Assets/Images/plin_golf7_wda.JPG)

# For ESP32 (Arduino IDE) aficionados
* [ESP32-SoftwareLIN](https://github.com/CW-B-W/ESP32-SoftwareLIN) (a nice read about the break field)

# What's inside?
* brushed DC motor
* H-bridge converter (four 90N04 N-Channel MOSFETs)
* magnetic field sensor (KMZ43T from NXP)
* uC (D1923BA from Bosch)
* LIN transceiver (TJA1021 from NXP)

![Bosch WDA PCB top](/Assets/Images/golf7_pcb_top.jpg)

![Bosch WDA PCB bottom](/Assets/Images/golf7_pcb_bottom.jpg)

![Bosch WDA uC](/Assets/Images/golf7_bosch_uc.jpg)

![Bosch WDA magnet](/Assets/Images/golf7_position_sensor_magnet.jpg)

![Bosch WDA cover 2](/Assets/Images/golf7_cover_with_socket_2.jpg)


# Call for action
Create your own [home laboratory/workshop/garage](http://ufnalski.edu.pl/control_engineering_for_hobbyists/2024_dzien_popularyzacji_matematyki/Dzien_Popularyzacji_Matematyki_2024.pdf)! Get inspired by [ControllersTech](https://www.youtube.com/@ControllersTech), [DroneBot Workshop](https://www.youtube.com/@Dronebotworkshop), [Andreas Spiess](https://www.youtube.com/@AndreasSpiess), [GreatScott!](https://www.youtube.com/@greatscottlab), [ElectroBOOM](https://www.youtube.com/@ElectroBOOM), [Phil's Lab](https://www.youtube.com/@PhilsLab), [atomic14](https://www.youtube.com/@atomic14), [That Project](https://www.youtube.com/@ThatProject), [Paul McWhorter](https://www.youtube.com/@paulmcwhorter), [Max Imagination](https://www.youtube.com/@MaxImagination), [Nikodem Bartnik](https://www.youtube.com/@nikodembartnik), and many other professional hobbyists sharing their awesome projects and tutorials! Shout-out/kudos to all of them!

> [!WARNING]
> Control engineering - do try this at home :sunglasses:

190+ challenges to start from: [Control Engineering for Hobbyists at the Warsaw University of Technology](http://ufnalski.edu.pl/control_engineering_for_hobbyists/Control_Engineering_for_Hobbyists_list_of_challenges.pdf).

Stay tuned!