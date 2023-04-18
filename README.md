# Mfilemon

Mfilemon is a print monitor for 32 and 64 bit MS Windows (XP and greater). It automates "print to file" jobs choosing the filename according to a pattern. It can redirect data to an external program (e.g. Ghostscript, to produce PDF).

## Sample usage

- Install [Mfilemon](https://github.com/lomo74/mfilemon/releases/latest)
- Install [Ghostscript](https://ghostscript.com/releases/gsdnld.html)
- Create a new printer. Choose manual configuration. Pick the driver from *ghostscript*\\lib\\ghostpdf.inf
- Create a new port for the printer, select "Multi file port", name it GSPDF: or whatever
- Configure the newly created port as follows:
	- Output path = C:\\autopdf
	- Filename pattern = %Y_%m_%d\\file%i.pdf
	- User command (change Ghostscript path as needed) = "C:\\Program Files\\gs\\gs9.53.3\\bin\\gswin64c.exe" @"C:\\Program Files\\mfilemon\\conf\\gspdf.conf" -sOutputFile="%f" -
	- Use pipe to send data to user command = yes
	- Hide process = yes
- Print a sample page. You should get a PDF in C:\\autopdf\\*year_month_day*\\file0001.pdf

## Important note for TSplus users

An incompatibility issue has been observed on systems running the TSplus software together with Mfilemon.
The problem is due to a naming conflict: both software use "mfilemon.dll" and "mfilemonui.dll" for the executable files and "Multi file port" as the port name.
For those who have problems, a custom version has been created that resolves the clash of names: download and install **amfilemon-setup.exe** instead of mfilemon-setup.exe, then choose "Auto multi file port" when creating the printer port.
