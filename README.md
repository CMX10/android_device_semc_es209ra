android_device_semc_es209ra
===========================

SE Xperia X10

How to build:
-------------

Initialize repo:

    repo init -u git://github.com/CyanogenMod/android.git -b cm-10.1
    curl -L -o .repo/local_manifests/local_manifest.xml -O -L https://raw.github.com/legaCyMod/android_local_manifest/cm-10.1/local_manifest.xml
    repo sync

edit local_manifest and add

	<?xml version="1.0" encoding="UTF-8"?>
	<manifest>
		...

		<remote name="bitbucket"
				fetch="ssh://git@bitbucket.org" />
	
	    <project name="CMX10/android_device_semc_es209ra" path="device/semc/es209ra" remote="bitbucket" revision="cm10.1" />
	    <project name="CMX10/proprietary_es209ra" path="vendor/semc/es209ra" revision="jellybean" />
	    <project name="CMX10/android_kernel_semc_es209ra" path="kernel/semc/es209ra" revision="master" />
	</manifest>

Compile:

. build/envsetup.sh && lunch cm_es209ra-userdebug
    make bacon -j8

Credits:

* Konstat repo (Zte Blade) for all the great work they've done and we took part of it to improve our device https://github.com/KonstaT
* Cyanogen Mod for the Base
* FXP for the initial device repository
