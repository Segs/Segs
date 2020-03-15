Frequently Asked Questions
======

Q. Who are you guys/gals?

A. Former Citizens of Paragon, just like you. We’re just trying to rebuild our 
online home, one line of code at a time. We volunteer our time and skills 
to get the job done


Q. Are you guys for real? Not an April Fools? 

A. Nope we’re real. We WERE going to announce on April 1st, but we waited an extra 
day because we didn’t want to come off as a prank. 


Q. For reals?!

A. Yes. 


Q: How is this different from ParagonChat? 

A: PChat appears to be focused on providing an interactive chat environment using 
the CoX client. The chat platform they're building on will make things like combat 
extremely difficult, if not impossible. They are also working with the I23+ client 
currently. The SEGS team, on the other hand, is building a true server, designed to 
interface with multiple CoX clients. This will eventually allow for AI enemies and 
combat. Currently focusing on Issue 0 client, but with plans to eventually move to 
a newer client.

From a technical perspective, PChat is not an authoritative server, meaning it does 
not care what you tell it you're doing. A clever "hacker" can tell the server that 
he's wearing a Statesman costume and flying upside-down at superspeed, and the PChat 
server will just assume that's ok and send it out to all other clients. This poses 
some obvious challenges for combat and competitive types of play. In simplistic terms, 
PChat is a chat server that interprets client information to send back and forth, it 
really doesn't try to be more (yet?). PChat appears to be closed source.

SEGS however IS an authoritative server. This is the reason why we're still working on 
rubberbanding issues. The server must know what your character is doing at all times, 
and needs to ensure that you're allowed to do that. This means that both the server 
and client need to calculate interpolation for movement the same way. It needs to 
ensure your costume is legitimate and that you have the powers you're using. It makes 
development much more complicated, but also means that we can eventually have the game 
we all love and remember. Because SEGS is an actual c++ developed server, it also 
means that we can eventually have combat and AI and missions and etc. 


Q: Are there any DMCA concerns from NCSoft for intellectual property? Are entities 
like Statesman and the like going to be genericized, much like player characters 
that too closely resembled The Hulk et all were back in the real game?

A: We don't think so. NPC definitions and such will either come from user 
supplied files, or converted client assets, and converted client assets are in the 
hands of the user base. We won't be distributing those. So pretty much like how 
Paragon Chat came about. And the clients were all freely available from NCSoft's own
FTP server when the game was still active. We haven't modified it, or changed it 
in any way. 


Q: So, is it possible to actually play City of Heroes?

A: As of right now, it's not's possible to play the full game. The current version of 
this server allows you to be able to log in create a toon and walk around Atlas Park. 
But the map doesn't know where anything or anyone is right now, so it will rubber 
band you. 


Q. How do I create my own server?

A. Well, it’s not a complete server. More like 5%. By if you want 
to try what we have, you can download a Windows EXE file, or compile the code using 
QT Creator with Qt 5.12+. Then using the I0 client, and an app to tell it what address 
to connect to, login, create a character and be in Atlas Park. More detailed 
instructions are in the Readme on our GitHub page. 

Q. How do I get the client and address app?

A. There's torrent you can get it from. The link for the torrent is in the Readme.
(Hint: reading the Readme file is VERY important. It has a lot of information.)


Q. Should I read the "Readme File?"

A. Yes. Several times. Commit it to memory. There will be a quiz next period. 


Q: How can I help you guys?

A: Well, if you know C++ and are comforatble with Object Oriented coding, OO for short, 
You can go to the GitHub page, and look at issues, and pick a task that fits your 
knowledge. Some of them are marked as "good first issue". If you're not a coder, you 
can still help by downloading the EXE or compiling the code for your platform, and 
running it, and sending in bug reports. Plus there are a few non-programmer issues 
you might be able to tackle as well. 


Q: How can I keep up with what is going on?

A: Follow us on Twitter (@segscode), stop by these forums often, or join us in chat. You 
can find the info for the chat and forums in the Readme on the GitHub page. 
http://github.com/Segs/Segs
