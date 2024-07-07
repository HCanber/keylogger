CC=gcc
CFLAGS=-framework ApplicationServices -framework Carbon -std=c99 $(FLAGS)

HOME_DIR := $(shell echo ~)
TMPDIR := build
BUILD_DIR=build
LOGDIR=$(HOME_DIR)/.keylogs/

INSTALLDIR=$(HOME_DIR)/bin
#INSTALLDIR=/usr/local/bin

# LaunchDaemons are system-wide and run independently of user sessions. They start when the system boots and stop when the system shuts down.
#PLISTDIR=/Library/LaunchDaemons

# LaunchAgents are loaded and run in the context of a user session, and they stop when the user logs out.
PLISTDIR=$(HOME_DIR)/Library/LaunchAgents

KEYLOGGER_SOURCE=keylogger.c
KEYLOGGER_EXECUTABLE=keylogger
KEYLOGGER_PLIST=keylogger.plist
KEYLOGGER_PLISTFULL=$(abspath $(PLISTDIR)/$(KEYLOGGER_PLIST))
KEYLOGGER_LOGFILE=$(abspath $(LOGDIR)/keystroke.log)
KEYLOGGER_LOGFILE_STDOUT=$(abspath $(LOGDIR)/$(KEYLOGGER_EXECUTABLE)_std.log)
KEYLOGGER_LOGFILE_STDERR=$(KEYLOGGER_LOGFILE_STDOUT)
KEYLOGGER_SERVICE_ID=keylogger


KEYCOUNTER_SOURCE=keycounter.c
KEYCOUNTER_EXECUTABLE=keycounter
KEYCOUNTER_PLIST=keycounter.plist
KEYCOUNTER_PLISTFULL=$(abspath $(PLISTDIR)/$(KEYCOUNTER_PLIST))
KEYCOUNTER_LOGFILE=$(abspath $(LOGDIR)/$(KEYCOUNTER_EXECUTABLE).csv)
KEYCOUNTER_LOGFILE_STDOUT=$(abspath $(LOGDIR)/$(KEYCOUNTER_EXECUTABLE)_std.log)
KEYCOUNTER_LOGFILE_STDERR=$(KEYCOUNTER_LOGFILE_STDOUT)
KEYCOUNTER_SERVICE_ID=keycounter

all: keylogger keycounter
install: keylogger_install keycounter_install
uninstall: keylogger_uninstall keycounter_uninstall
startup: keylogger_startup keycounter_startup
load: keylogger_load keycounter_load
unload: keylogger_unload keycounter_unload
clean: keylogger_clean keycounter_clean

keylogger: $(KEYLOGGER_SOURCE)
	$(CC) $(KEYLOGGER_SOURCE) $(CFLAGS) -o $(BUILD_DIR)/$(KEYLOGGER_EXECUTABLE)

keylogger_install: keylogger
	mkdir -p $(INSTALLDIR)
	cp $(BUILD_DIR)/$(KEYLOGGER_EXECUTABLE) $(INSTALLDIR)

keylogger_uninstall:
	launchctl unload $(KEYLOGGER_PLISTFULL)
	rm $(INSTALLDIR)/$(KEYLOGGER_EXECUTABLE)
	rm -f $(KEYLOGGER_PLISTFULL)

keylogger_startup: keylogger_install
	@cp -f "plist.TEMPLATE" "$(TMPDIR)/$(KEYLOGGER_PLIST)"
	@sed -i '' \
		-e "s|{LABEL}|$(KEYLOGGER_SERVICE_ID)|g" \
		-e "s|{EXE}|$(INSTALLDIR)/$(KEYLOGGER_EXECUTABLE)|g" \
		-e "s|{LOGFILE}|$(KEYLOGGER_LOGFILE)|g" \
		-e "s|{LOGFILE_STDOUT}|$(KEYLOGGER_LOGFILE_STDOUT)|g" \
		-e "s|{LOGFILE_STDERR}|$(KEYLOGGER_LOGFILE_STDERR)|g" \
		"$(TMPDIR)/$(KEYLOGGER_PLIST)"
	@mkdir -p $(shell dirname "$(KEYLOGGER_LOGFILE)")
	@mkdir -p $(shell dirname "$(KEYLOGGER_LOGFILE_STDOUT)")
	@mkdir -p $(shell dirname "$(KEYLOGGER_LOGFILE_STDERR)")
	mv "$(TMPDIR)/$(KEYLOGGER_PLIST)" $(KEYLOGGER_PLISTFULL)
	launchctl load -w $(KEYLOGGER_PLISTFULL)
	@echo "Service $(KEYLOGGER_SERVICE_ID) started"

keylogger_load:
	launchctl load $(KEYLOGGER_PLISTFULL)

keylogger_unload:
	launchctl unload $(KEYLOGGER_PLISTFULL)

keylogger_clean:
	rm -f $(BUILD_DIR)/$(KEYLOGGER_EXECUTABLE)


keycounter: $(KEYCOUNTER_SOURCE)
	$(CC) $(KEYCOUNTER_SOURCE) $(CFLAGS) -o $(BUILD_DIR)/$(KEYCOUNTER_EXECUTABLE)

keycounter_install: keycounter
	@mkdir -p $(INSTALLDIR)
	@cp $(BUILD_DIR)/$(KEYCOUNTER_EXECUTABLE) $(INSTALLDIR)
	@echo "keycounter copied to $(INSTALLDIR)/$(KEYCOUNTER_EXECUTABLE)"

keycounter_uninstall:
	launchctl unload $(KEYCOUNTER_PLISTFULL)
	rm $(INSTALLDIR)/$(KEYCOUNTER_EXECUTABLE)
	rm -f $(KEYCOUNTER_PLISTFULL)

keycounter_startup: keycounter_install
	@cp -f "plist.TEMPLATE" "$(TMPDIR)/$(KEYCOUNTER_PLIST)"
	@sed -i '' \
		-e "s|{LABEL}|$(KEYCOUNTER_SERVICE_ID)|g" \
		-e "s|{EXE}|$(INSTALLDIR)/$(KEYCOUNTER_EXECUTABLE)|g" \
		-e "s|{LOGFILE}|$(KEYCOUNTER_LOGFILE)|g" \
		-e "s|{LOGFILE_STDOUT}|$(KEYCOUNTER_LOGFILE_STDOUT)|g" \
		-e "s|{LOGFILE_STDERR}|$(KEYCOUNTER_LOGFILE_STDERR)|g" \
		"$(TMPDIR)/$(KEYCOUNTER_PLIST)"
	@mkdir -p $(shell dirname "$(KEYCOUNTER_LOGFILE)")
	@mkdir -p $(shell dirname "$(KEYCOUNTER_LOGFILE_STDOUT)")
	@mkdir -p $(shell dirname "$(KEYCOUNTER_LOGFILE_STDERR)")
	mv "$(TMPDIR)/$(KEYCOUNTER_PLIST)" $(KEYCOUNTER_PLISTFULL)
	launchctl load -w $(KEYCOUNTER_PLISTFULL)
	@echo "Service $(KEYCOUNTER_SERVICE_ID) started"

keycounter_load:
	launchctl load $(KEYCOUNTER_PLISTFULL)

keycounter_unload:
	launchctl unload $(KEYCOUNTER_PLISTFULL)

keycounter_clean:
	rm -f $(BUILD_DIR)/$(KEYCOUNTER_EXECUTABLE)
