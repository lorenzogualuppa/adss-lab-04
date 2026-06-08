EASYLOCAL = ../easylocal-3
FLAGS = -Wall -Wfatal-errors -Wno-sign-compare -Wno-deprecated-declarations -O3 -std=c++17
COMPOPTS = -I$(EASYLOCAL)/include -I/opt/homebrew/include -I/usr/local/include $(FLAGS)
LINKOPTS = -L/opt/homebrew/lib -L/usr/local/lib -lboost_program_options -pthread

SOURCE_FILES = WL_Data.cc WL_Helpers.cc  WL_Main.cc
OBJECT_FILES = WL_Data.o WL_Helpers.o WL_Main.o
HEADER_FILES = WL_Data.hh WL_Helpers.hh  

wlp: $(OBJECT_FILES)
	g++ $(OBJECT_FILES) $(LINKOPTS) -o wlp

WL_Data.o: WL_Data.cc WL_Data.hh
	g++ -c $(COMPOPTS) WL_Data.cc

WL_Helpers.o: WL_Helpers.cc WL_Helpers.hh WL_Data.hh
	g++ -c $(COMPOPTS) WL_Helpers.cc

WL_Main.o: WL_Main.cc WL_Helpers.hh WL_Data.hh
	g++ -c $(COMPOPTS) WL_Main.cc

clean:
	rm -f $(OBJECT_FILES) wlp