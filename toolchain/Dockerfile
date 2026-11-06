# The scmbot toolchain: build a BWAPI 4.4.0 bot (an AI module, C++) into one .scmbot file that scm-sim's play page
# (scmjs.dev/play) loads. Emscripten; BWAPI 4.4.0's in-game source (bwapi/, LGPL-3), compiled once; the Windows
# stand-ins (shim/) and the host that stands in for BWAPI.dll's Windows side (host/); and the scmbot script.
# Published as ghcr.io/scm-js/scmbot from github.com/scm-js/scmbot, whose root is this folder (native/ in scm-sim).
#
#   toolchain/scmbot <command>                        (in the bot's folder: the image of this toolchain, downloaded or built)
#   docker build -f toolchain/Dockerfile -t scmbot .  (by hand, from the folder holding toolchain/, shim/, host/, bwapi/)
#   docker run --rm -v "$PWD":/bot -u "$(id -u):$(id -g)" ghcr.io/scm-js/scmbot build      (in the bot's folder)
#   docker run --rm -v "$PWD":/bot -u "$(id -u):$(id -g)" ghcr.io/scm-js/scmbot init       (a scmbot.json to start from)
#
# The Emscripten image is pinned: the module's memory layout and its loader have to be what the page expects.
FROM emscripten/emsdk@sha256:fb7864e398acfda863dfcea78b8bc8adad359fa60c6a0bbeb845931de64aa68a
ENV SCMBOT_HOME=/scmbot SCMBOT_OBJ=/scmbot/obj
COPY bwapi /scmbot/bwapi
COPY shim /scmbot/shim
COPY host /scmbot/host
COPY toolchain/scmbot.mjs toolchain/msvc-compat.h /scmbot/toolchain/
RUN node /scmbot/toolchain/scmbot.mjs prepare && chmod -R a+rX /scmbot
# The cache Emscripten fills on first use, writable by whoever runs the build (-u).
RUN chmod -R a+rwX /emsdk/upstream/emscripten/cache
WORKDIR /bot
ENTRYPOINT ["node", "/scmbot/toolchain/scmbot.mjs"]
CMD ["build"]
