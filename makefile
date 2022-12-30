.SILENT:

All:  CreationBD Client Serveur Gerant Publicite Caddie 

CreationBD:	CreationBD.cpp
		echo -e "\033[91mCreation de la base de donnée...\033[0m"
		g++ -o CreationBD CreationBD.cpp -I/usr/include/mysql -m64 -L/usr/lib64/mysql -lmysqlclient -lpthread -lz -lm -lrt -lssl -lcrypto -ldl

Publicite:	Publicite.cpp
		echo -e "\033[91mCreation Publicite...\033[0m"
		g++ -o Publicite Publicite.cpp

Caddie:	Caddie.cpp
		echo -e "\033[91mCreation Caddie...\033[0m"
		g++ -o Caddie Caddie.cpp -I/usr/include/mysql -m64 -L/usr/lib64/mysql -lmysqlclient -lpthread -lz -lm -lrt -lssl -lcrypto -ldl

AccesBD:	AccesBD.cpp
		echo -e "\033[91mCreation des AccesBD...\033[0m"
		g++ -o AccesBD AccesBD.cpp -I/usr/include/mysql -m64 -L/usr/lib64/mysql -lmysqlclient -lpthread -lz -lm -lrt -lssl -lcrypto -ldl

maingerant.o:	maingerant.cpp
		echo -e "\033[94mCreation maingerant.o...\033[0m"
		g++ -Wno-unused-parameter -c -pipe -g -std=gnu++11 -Wall -W -D_REENTRANT -fPIC -DQT_DEPRECATED_WARNINGS -DQT_QML_DEBUG -DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_CORE_LIB -I../Administrateur -I. -isystem /usr/include/qt5 -isystem /usr/include/qt5/QtWidgets -isystem /usr/include/qt5/QtGui -isystem /usr/include/qt5/QtCore -I. -I. -I/usr/lib64/qt5/mkspecs/linux-g++ -o maingerant.o maingerant.cpp

windowgerant.o:	windowgerant.cpp
		echo -e "\033[94mCreation windowgerant.o...\033[0m"
		g++ -Wno-unused-parameter -c -pipe -g -std=gnu++11 -Wall -W -D_REENTRANT -fPIC -DQT_DEPRECATED_WARNINGS -DQT_QML_DEBUG -DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_CORE_LIB -I../Administrateur -I. -isystem /usr/include/qt5 -isystem /usr/include/qt5/QtWidgets -isystem /usr/include/qt5/QtGui -isystem /usr/include/qt5/QtCore -I. -I. -I/usr/lib64/qt5/mkspecs/linux-g++ -I/usr/include/mysql -m64 -L/usr/lib64/mysql -o windowgerant.o windowgerant.cpp

moc_windowgerant.o:	moc_windowgerant.cpp
		echo -e "\033[94mCreation moc_windowgerant.o...\033[0m"
		g++ -Wno-unused-parameter -c -pipe -g -std=gnu++11 -Wall -W -D_REENTRANT -fPIC -DQT_DEPRECATED_WARNINGS -DQT_QML_DEBUG -DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_CORE_LIB -I../Administrateur -I. -isystem /usr/include/qt5 -isystem /usr/include/qt5/QtWidgets -isystem /usr/include/qt5/QtGui -isystem /usr/include/qt5/QtCore -I. -I. -I/usr/lib64/qt5/mkspecs/linux-g++ -o moc_windowgerant.o moc_windowgerant.cpp

Gerant:	maingerant.o windowgerant.o moc_windowgerant.o
		echo -e "\033[91mCreation Gerant...\033[0m"
		g++ -Wno-unused-parameter -o Gerant maingerant.o windowgerant.o moc_windowgerant.o /usr/lib64/libQt5Widgets.so /usr/lib64/libQt5Gui.so /usr/lib64/libQt5Core.so /usr/lib64/libGL.so -L/usr/lib64/mysql -lmysqlclient -lpthread -lz -lm -lrt -lssl -lcrypto -ldl

mainclient.o:	mainclient.cpp
		echo -e "\033[94mCreation mainclient.o...\033[0m"
		g++ -Wno-unused-parameter -c -pipe -g -std=gnu++11 -Wall -W -D_REENTRANT -fPIC -DQT_DEPRECATED_WARNINGS -DQT_QML_DEBUG -DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_CORE_LIB -I../UNIX_DOSSIER_FINAL -I. -isystem /usr/include/qt5 -isystem /usr/include/qt5/QtWidgets -isystem /usr/include/qt5/QtGui -isystem /usr/include/qt5/QtCore -I. -I. -I/usr/lib64/qt5/mkspecs/linux-g++ -o mainclient.o mainclient.cpp

windowclient.o:	windowclient.cpp
		echo -e "\033[94mCreation windowclient.o...\033[0m"
		g++ -Wno-unused-parameter -c -pipe -g -std=gnu++11 -Wall -W -D_REENTRANT -fPIC -DQT_DEPRECATED_WARNINGS -DQT_QML_DEBUG -DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_CORE_LIB -I../UNIX_DOSSIER_FINAL -I. -isystem /usr/include/qt5 -isystem /usr/include/qt5/QtWidgets -isystem /usr/include/qt5/QtGui -isystem /usr/include/qt5/QtCore -I. -I. -I/usr/lib64/qt5/mkspecs/linux-g++ -o windowclient.o windowclient.cpp

moc_windowclient.o:	moc_windowclient.cpp
		echo -e "\033[94mCreation moc_windowclient.o...\033[0m"
		g++ -Wno-unused-parameter -c -pipe -g -std=gnu++11 -Wall -W -D_REENTRANT -fPIC -DQT_DEPRECATED_WARNINGS -DQT_QML_DEBUG -DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_CORE_LIB -I../UNIX_DOSSIER_FINAL -I. -isystem /usr/include/qt5 -isystem /usr/include/qt5/QtWidgets -isystem /usr/include/qt5/QtGui -isystem /usr/include/qt5/QtCore -I. -I. -I/usr/lib64/qt5/mkspecs/linux-g++ -o moc_windowclient.o moc_windowclient.cpp

Client:	mainclient.o windowclient.o moc_windowclient.o
		echo -e "\033[91mCreation Client...\033[0m"
		g++ -Wno-unused-parameter -o Client mainclient.o windowclient.o moc_windowclient.o  /usr/lib64/libQt5Widgets.so /usr/lib64/libQt5Gui.so /usr/lib64/libQt5Core.so /usr/lib64/libGL.so -lpthread

FichierClient.o:	FichierClient.cpp
		echo -e "\033[94mCreation FichierClient.o...\033[0m"
		g++ -o FichierClient.o -c FichierClient.cpp

Serveur:	Serveur.cpp FichierClient.o
		echo -e "\033[91mCreation Serveur...\033[0m"
		g++ -std=c++2a Serveur.cpp FichierClient.o -o Serveur -I/usr/include/mysql -m64 -L/usr/lib64/mysql -lmysqlclient -lpthread -lz -lm -lrt -lssl -lcrypto -ldl


clean:
		rm *.o Client Serveur Gerant Publicite Caddie 