bin:
	cd src; make bin

lib:
	cd src; make lib

all:
	cd src; make all

clean:
	cd src; make clean

clear:
	cd src; make clear

####### Platform-specific targets

deb: bin distrib/finddecryptor.equivs
	equivs-build distrib/finddecryptor.equivs
