#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>

#include "VideoFile.h"
#include "BlackBoard.h"

using namespace std;

int main (int argc, char* argv[]){

	try{
	//TODO cambiar esto para recibirlo por parametros.
	cout<<"Creating the black board"<<endl;
	BlackBoard blackBoard("D:\\Dropbox\\Facultad\\ComputerVision\\videos\\bird.avi");

	cout<<"Starting the black board"<<endl;
	blackBoard.load();

	cout<<"Starting the black board"<<endl;
	blackBoard.start();
	}
	catch (char * s)
	{
		cout<<"There was a problem :(."<<endl;
		cout<<"Error's text: "<<s<<endl;
	}
	catch (...)
	{
		cout<<"There was a problem :(."<<endl;
	}
	system("pause");
}