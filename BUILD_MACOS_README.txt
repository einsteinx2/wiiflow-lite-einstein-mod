WiiFlow Lite Compilation notes High Sierra:

1. Install devkitpro ppc
2. Add /opt/devkitpro/devkitPPC/bin to your PATH
3. Install libsicksaxis (clone https://github.com/xerpi/libsicksaxis, make, make install)
4. Use homebrew to install GNU coreutils: brew install core-utils
5. Add GNU's utils to PATH so that they override the included BSD versions (mostly needed for mv -u I think): PATH="$(brew --prefix coreutils)/libexec/gnubin":$PATH
6. Build! (just run make)
