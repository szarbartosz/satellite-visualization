#ifdef _REQUEST_API
std::string apiKey = readApiKeyFromFile("apikey.txt");
std::string apiURL = "https://api.n2yo.com/rest/v1/satellite//tle/25544&apiKey=" + apiKey;
std::string response;

std::cout << "Fetching data from API...\n";
try {
	curlpp::Cleanup cleaner;
	curlpp::Easy request;

	request.setOpt(new curlpp::options::Url(apiURL));
	std::ostringstream stream;
	curlpp::options::WriteStream ws(&stream);
	request.setOpt(ws);

	// Perform the request
	request.perform();

	// Get the response from the stream
	response = stream.str();
}
catch (curlpp::LogicError& e) {
	std::cout << e.what() << std::endl;
}
catch (curlpp::RuntimeError& e) {
	std::cout << e.what() << std::endl;
}

Json::Value root;
Json::Reader reader;
reader.parse(response, root);
TLEs.push_back(root["tle"].asString());

#undef _REQUEST_API
#endif