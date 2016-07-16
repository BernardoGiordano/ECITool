# EventAssistant
A general purpose tool mostly oriented to event collectors.

**[EventAssistant](https://github.com/BernardoGiordano/EventAssistant/releases): what is this?**

[**EventAssistant**](https://github.com/BernardoGiordano/EventAssistant/releases) is a general purpose pokémon application that can be run on your 3DS or PC (you need *Homebrew Launcher* to launch the *.3dsx* version or EmuNAND to run the *.cia* version on the 3DS, or Citra to run the *.3dsx* version on your PC).

This application allows you to make some cool things, like:

* Checking **PID** of a pokémon. Pretty useful to check very old events, that may be manipulated with old tools and could have leftovers of manipulation. You can see if calculations done by the app are coherent with the pokémon.
* Seeing **current event distributions** directly from your 3DS. It provides infos about start and end dates of local and wifi distributions, with related codes if there is a general code for the distribution.
* **Capture probability calculation**: you can now calculate it directly on your 3DS, for every generation starting by the third. You can also calculate how much tries are needed to reach a certain probability of success.
* Check **Powersaves dates** for common Hacked Events;
* Check **Gen VI's Event Database**. 

**Important things about Gen VI's Event Database**

Event Database links are hardcoded in the program, so it knows where to look for stuff. The point is that writing this stuff is very difficult to do alone, so lots of event cards are unavailable at the moment. They'll be available when uploaded on the server I use, and you'll be able to see them without any update of the app. Updates are needed if new events are released, basically to update the description of the event.


**How to use it on my PC instead of 3DS?**

You can use Citra to run the *.3dsx* version of the app. I made a few changes to have a better usability with Citra. As for now, you can only use properly PID Checker and Capture Probability Calculator, because http functions are unimplemented on Citra. To scroll properly values on Citra, just set the program in **SLOW mode**.

**What will come next?**

* **.wc6 download and injection directly from the 3DS**
* Much more informations related to PID checking;
* Capture probability calculation support for I-II gens;
* Regular updates to the Event Database.

Tell me if you like this work and what you would have featured in the next update! :)
 
*Thanks Shai for the icon and Fabrizio for the name!*

# How to compile it?

Use latest **ctrulib** and devKitArm.