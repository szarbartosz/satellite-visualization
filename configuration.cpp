#ifdef _CONFIGURATION
	
	windowWidth = 800;
	windowHeight = 600;
	
	isFullScreen = false;
	
	setupCameraArea(500);

	
	std::string apiKey = readApiKeyFromFile("apikey.txt");
	std::cout << "API key: " << apiKey << std::endl;

	std::cout << "Fetching data from api...\n";
	try {
		curlpp::Cleanup cleaner;
		curlpp::Easy request;

		//// Setting the URL to retrive.
		//request.setOpt(new curlpp::options::Url(std::string("https://api.n2yo.com/rest/v1/satellite//tle/25544&apiKey=" + apiKey)));

		//std::cout << "Fetched data: " << request << std::endl;

		// Even easier version. It does the same thing 
		// but if you need to download only an url,
		// this is the easiest way to do it.
		std::cout << curlpp::options::Url(std::string("https://api.n2yo.com/rest/v1/satellite//tle/25544&apiKey=" + apiKey)) << std::endl;
	}
	catch (curlpp::LogicError& e) {
		std::cout << e.what() << std::endl;
	}
	catch (curlpp::RuntimeError& e) {
		std::cout << e.what() << std::endl;
	}

#undef _CONFIGURATION
#endif
