# FTP Internet Procotol for Server
Implementation of FTP Internet Procotol for Server. 

Description
--------------
Boost.Asio and Boost.Filesystem is library I mainly used to create project.
Protocol is founded on RFC 959 and other updated versions.
In the next versions I am going to increase effectiveness,portability,stability and performance.Especially upgrade protocol to "SSH File Transfer Protocol" and implement new availible commands.

How it works
--------------
Every launch of the program was performed on the Filezilla as client.
Server allows us to listing directory(root directory is started from launch directory of main .exe) files and download them.
Additionally,server is based on the "asynchronous" philosophy.


![Preview](https://i.imgur.com/oK3HcaE.jpg)

And example from Filezilla :
![Preview](https://i.imgur.com/qm30wAn.png)
