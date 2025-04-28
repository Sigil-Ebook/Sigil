# <center>Creating a Virtual Python Environment on Linux to Run Sigil Plugins</center>

Maybe you don't want to install a bunch of extra python modules in your system Python, or maybe your distro doesn't have pre-packaged versions of all the modules Sigil needs in their repositories (and/or maybe your distro won't allow you to "pip install" modules from [pypi.org](https://pypi.org) into your system Python because they think they know better than you about how to do things). Regardless of the reasons, this is the document to help you create a virtual Python environment that Sigil can use to run [useful 3rd-party plugins](https://www.mobileread.com/forums/forumdisplay.php?f=268).

This document assumes that the virtual environment is going to be created in your home directory in a folder named 'sigilpy' with all of the modules that are listed in the plugin-reqs.txt file found in the 'docs' folder of Sigil's source code. If you're not building Sigil from source and still want to create a virtual python to run plugins, you can [download/copy the plugin-reqs.txt file](https://github.com/Sigil-Ebook/Sigil/blob/master/docs/plugin-reqs.txt) from Sigil's github repository.

Copy/Save/Create the plugin-reqs.txt to your home directory

Open a terminal in your home directory and create the sigilpy virtual environment with the following command:

>`python -m venv sigilpy`

Activate the new virtual environment with the following command:

>`source sigilpy/bin/activate`

Update the pip module in the virtual environement to the latest version (from pypi.org):

>`python -m pip install --upgrade pip`

Install the modules listed in the plugin-reqs.txt file into the virtual environment with pip:

> `python -m pip install -r plugin-reqs.txt`

You don't need to keep the virtual environment activated after this point to use it for Sigil plugins so feel free to deactivate it by typing:

> `deactivate`

That's it. Now open Sigil's Preferences, go to the Plugin section, click the 'Set' button, browse to /home/[username]/sigilpy/bin/ and select the python binary. Make sure the latest testplugin is installed and use it to verify all tests are passing. Or if you don't have Sigil compiled yet, go back to the build instructions and configure Sigil's plugin preferences after you have Sigil up and running.
