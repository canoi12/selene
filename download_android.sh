CMDLT_FILE="commandlinetools-linux-11076708_latest.zip"
CMD_LINE_TOOLS_URL="https://dl.google.com/android/repository/$CMDLT_FILE"
if [ ! -f $CMDLT_FILE ]; then
    wget $CMD_LINE_TOOLS_URL
fi
unzip "$CMDLT_FILE" -d ".tmp/"