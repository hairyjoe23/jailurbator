include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Woverloaded-virtual -Wall -Wno-sign-compare -Werror")

set(SOURCES
  application.cpp
  browsingsettingspage.cpp
  closure.cpp
  database.cpp
  iconloader.cpp
  image.cpp
  imagebackend.cpp
  imagelistdelegate.cpp
  imagestate.cpp
  logging.cpp
  main.cpp
  mainwindow.cpp
  network.cpp
  redditloginsettingspage.cpp
  redditmodel.cpp
  scopedtransaction.cpp
  settingsdialog.cpp
  settingspage.cpp
  smoothscrollinglistview.cpp
  stylesheetloader.cpp
  subredditmodel.cpp
  subredditsettingspage.cpp
  utilities.cpp
)

set(HEADERS
  application.h
  browsingsettingspage.h
  closure.h
  database.h
  imagestate.h
  mainwindow.h
  network.h
  redditloginsettingspage.h
  redditmodel.h
  settingsdialog.h
  settingspage.h
  smoothscrollinglistview.h
  stylesheetloader.h
  subredditsettingspage.h
)

set(UI
  browsingsettingspage.ui
  imagestate.ui
  mainwindow.ui
  redditloginsettingspage.ui
  settingsdialog.ui
  subredditsettingspage.ui
)

set(RESOURCES
  ${CMAKE_SOURCE_DIR}/data/data.qrc
)

qt4_wrap_cpp(MOC ${HEADERS})
qt4_wrap_ui(UIC ${UI})
qt4_add_resources(QRC ${RESOURCES})


add_executable(redditimages
  ${SOURCES}
  ${MOC}
  ${UIC}
  ${QRC}
)

target_link_libraries(redditimages
  ${QT_LIBRARIES}
  ${QJSON_LIBRARIES}
)
