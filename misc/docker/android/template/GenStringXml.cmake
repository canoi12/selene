# configure_file("${CMAKE_CURRENT_LIST_DIR}/strings.xml.in" "${CMAKE_CURRENT_LIST_DIR}/app/src/main/res/values/strings.xml")
file(WRITE "${CMAKE_CURRENT_LIST_DIR}/app/src/main/res/values/strings.xml" "<resources>
    <string name=\"app_name\">$ENV{SELENE_APP_NAME}</string>
</resources>")