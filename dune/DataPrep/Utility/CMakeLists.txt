# dunetpc/dune/DataPrep/Utility/CMakeLists.txt

art_make(BASENAME_ONLY
         LIB_LIBRARIES
         dune_ArtSupport
         dune_DuneCommon_Utility
         art::Utilities
         canvas::canvas
         cetlib::cetlib
	 cetlib_except::cetlib_except
	 fhiclcpp
         ROOT_BASIC_LIB_LIST
         NO_PLUGINS
        )

install_source()
install_headers()

add_subdirectory(test)
