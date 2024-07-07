
# macOS key logger and counter

This is a fork of [caseyscarborough/keylogger](https://github.com/caseyscarborough/keylogger) with the following changes:

- keylogger: See below for changes
- keycounter: new executable, see below
- Build to `./build` and use it as tempdir for gcc. This folder can then be set up as an exception in your antivirus software, as it otherwise (hopefully) would be identified and blocked as a keylogger.

## keylogger

Logs key presses to a file. Read below under _Original README_ for more information.
The following changes have been made compared to the original fork:

- Log space as `[space]` instead of ``.
- Log `[fn]` key presses.
- Log unknown keys as `{keycode}` with the actual keycode instead of `[unknown]`.

## keycounter

If you just want to count the times different keys are pressed, for example for optimizing key layouts for custom keyboards, `keycounter` is better to use from a security perspecitve. It counts the key presses and logs them to a file.
For every keycode an array of 16 counts are logged, one for each of of the following modifiers:

| Index | Shift | Control | Option | Command |
|------:|:-----:|:-------:|:------:|:-------:|
| 0     |       |         |        |         |
| 1     | ⬜     |         |        |         |
| 2     |       | ⬜      |        |         |
| 3     | ⬜    | ⬜      |        |         |
| 4     |       |         | ⬜     |         |
| 5     | ⬜    |         | ⬜     |         |
| 6     |       | ⬜      | ⬜     |         |
| 7     | ⬜    | ⬜      | ⬜     |         |
| 8     |       |         |        | ⬜      |
| 9     | ⬜    |         |        | ⬜      |
| 10    |       | ⬜      |        | ⬜      |
| 11    | ⬜    | ⬜      |        | ⬜      |
| 12    |       |         | ⬜     | ⬜      |
| 13    | ⬜    |         | ⬜     | ⬜      |
| 14    |       | ⬜      | ⬜     | ⬜      |
| 15    | ⬜    | ⬜      | ⬜     | ⬜      |
| Index | Shift | Control | Option | Command |

## Prerequisites

Clone this repository and build:

```sh
git clone https://github.com/hcanber/keylogger && cd keylogger
make
```

## Build and run from command line

This will build both `keylogger` and `keycounter`.

To start `keycounter`:

```sh
build/keycounter
```

By default a csv file will be written in the build folder with the counts. You can change the path to the log file by passing it as an argument.

To start `keycounter` with path to log file:

```sh
build/keycounter path/to/logfile.csv
```

To save it as json, specify `--json` as an argument:

```sh
build/keycounter --json
```

## Install as service that runs on startup

```sh
make keycounter_startup
```

This will build, install it to `~/bin/keycounter` and create a launchd service that runs it on startup, and start it.
You'l be asked for permission, if not go to `System Preferences -> Security & Privacy -> General` and allow the service `keycounter` to run.

Log files will be written to `~/.keylogs/` (both key counts file and stdout/stderr files).

Note! if you rerun `make keycounter_startup` the new service sometimes will not get correct permissions to run. You need to go into `System Preferences -> Security & Privacy -> General`, select `keycounter` and click on the `-` button to remove it.

### Uninstall service

```sh
make keycounter_uninstall
```

----

# Original README

The original README from [caseyscarborough/keylogger](https://github.com/caseyscarborough/keylogger)
I've just modified the instructions for the build and installation process as I've updated the build process
----

# macOS Keylogger

This repository holds the code for a simple and easy to use keylogger for macOS. It is not meant to be malicious, and is written as a proof of concept. There is not a lot of information on keyloggers or implementing them on macOS, and most of the ones I've seen do not work as indicated. This project aims to be a simple implementation on how it can be accomplished on OS X.

> Note: This keylogger is currently unable to capture secure input such as passwords. See issue #3 for more information.

## Usage

Start by cloning the repository and running the proper make commands, shown below. By default, the application installs to `/usr/local/bin/keylogger`, which can easily be changed in the [`Makefile`](https://github.com/caseyscarborough/keylogger/blob/master/Makefile). `make install_keylogger` may require root access.

```bash
git clone https://github.com/hcanber/keylogger && cd keylogger
make && make keylogger_install
```

The application by default logs to `/var/log/keystroke.log`, which may require root access depending on your system's permissions. You can change this in [`keylogger.h`](https://github.com/caseyscarborough/keylogger/blob/master/keylogger.h#L12) if necessary.

```bash
$ keylogger
Logging to: /var/log/keystroke.log
```

If only modifier keys are logging (e.g. in macOS ≥ 10.10), run with root access.

If you'd like the application to run in the background on startup, run the `startup` make target:

```bash
sudo make keylogger_startup
```

To run the application now (note: you will need to run the `sudo make startup` command first):

```bash
sudo make keylogger_load
```

To quit the application now (note: you will need to run the `sudo make startup` command first)::

```bash
sudo make keylogger_unload
```

## Uninstallation

You can completely remove the application from your system (including the startup daemon) by running the following command (logs will not be deleted):

```bash
sudo make keylogger_uninstall
```

### Optional Parameters

You can pass in two optional parameters to the program. The `clear` option will clear the logs at the default location. Any other argument passed in will be used as the path to the log file for that process. See below:

```bash
# Clear the logfile.
$ keylogger clear
Logfile cleared.

# Specify a logfile location.
$ keylogger ~/logfile.txt
Logging to: /Users/Casey/logfile.txt
```

## Issues

### Unable to Create Event Tap

If you get the following error:

```
ERROR: Unable to create event tap.
```

Go into System Preferences and go to Security & Privacy, click the Privacy tab, choose Accessibility in the left pane, and ensure that Terminal is checked.

## Contributing

Feel free to fork the project and submit a pull request with your changes!
