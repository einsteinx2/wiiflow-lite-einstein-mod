# WiiFlow Lite Einstein Mod

Upstream repo for WiiFlow Lite by fledge68: https://sourceforge.net/projects/wiiflow-lite

The purpose of this repo and mod is to have a place to freely work on my own modifications and improvements to WiiFlow Lite. I will closely track upstream and merge in any new revisions to stay up to date and hopefully my changes will be merged upstream as well.

I've changed APP_NAME to WiiFlow Lite Einstein Mod and added my name to DEVELOPERS alongside fledge68. Also I've removed the SVN revision script along with the .svn folder since I’m not using SVN. I’ve decided to use my own versioning system starting at build 1 to better track my own changes and avoid confusion, so I'll just be manually updating the source/svnrev.h file as I want to update build numbers. I've decided to leave the SVN_REV variable name alone to prevent other source code changes. I’ll always note in the commit what the current SVN version is when I merge in new changes. I’ll keep the APP_VERSION the same as whatever upstream WiiFlow Lite has set.

I’ll attempt to keep my changes mergable (i.e. not make drastic refactorings, change brace style, variable naming conventions, etc). Also, I’ll attempt to keep compatibilty with the wiiflow_lite.ini file by only adding options, never renaming existing ones. Same goes for compatiblity with the wiiflow settings directory.

However, I will change this mod in an upcoming commit so that it runs in it’s own wiiflow_einstein directory with it’s own ini file, so it can be used alongside upstream WiiFlow Lite, however it will share the wiiflow settings directory. I just need to get a forwarder channel and priiloader launcher built first. In the meantime, it will work as a drop in replacement for WiiFlow Lite. If there's demand, I can also create alternative builds that use the wiiflow_lite and wiiflow folders for compatibilty with existing channels and launchers.
