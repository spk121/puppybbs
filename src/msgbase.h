#pragma once
#include "puppy.h"
void closemsg();
int ismsg(int n, int t);
void openmsg();
int oldest_msg();
int newest_msg();
void listhdr(int n);
void listmsg(int n);
WORD alltopics();
void loadmsg(int n);
void topics(WORD t);
void savemsg(int lines);
