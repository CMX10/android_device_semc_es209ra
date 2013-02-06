android_device_semc_es209ra
===========================

SE Xperia X10

How to build:
-------------

Initialize CyanogenMod repo:

    repo init -u git://github.com/CyanogenMod/android.git -b jellybean
    repo sync

Add the following to your cm10/.repo/local_manifest.xml and repo sync again:

	<?xml version="1.0" encoding="UTF-8"?>
	<manifest>
		<remote name="bitbucket"
				fetch="ssh://git@bitbucket.org" />
	
	    <project name="CMX10/android_device_semc_es209ra" path="device/semc/es209ra" remote="bitbucket" revision="jellybean" />
	    <project name="CMX10/proprietary_es209ra" path="vendor/semc/es209ra" revision="jellybean" />
	    <project name="CMX10/android_kernel_semc_es209ra" path="kernel/semc/es209ra" revision="master" />
	</manifest>


Credits:

* Konstat repo (Zte Blade) for all the great work they've done and we took part of it to improve our device https://github.com/KonstaT
* Cyanogen Mod for the Base
* FXP for the initial device repository
