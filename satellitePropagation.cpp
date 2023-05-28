#ifdef _SATELLITE_PROPAGATION

	vector<int> currentTimeV = getCurrentTimeV();

	if (prevSecond != (int)(currentTimeV[6] / 100)) {
		prevSecond = (int)(currentTimeV[6] / 100);

		satellitePositions.clear();

		for (int i = 0; i < satellitePositionsFrom2Sec.size(); i++) {
			satellitePositionsFrom2Sec[i].clear();
		}

		interpolationIncrement += 0.25;

		for (int i = 0; i < TLEs.size(); i++) {
			vector<SatellitePos> satellitePosVector;
			satellitePositionsFrom2Sec.push_back(satellitePosVector);
			

			std::string delimiter = "\r\n";
			std::string line1 = TLEs[i].substr(0, TLEs[i].find(delimiter));
			std::string line2 = TLEs[i].substr(TLEs[i].find(delimiter)+2, TLEs[i].length());
			
			// Create a TLE object from the two lines of text
			Tle tle(line1, line2);

			// Create an SGP4 propagator object
			SGP4 sgp4(tle);

			int secondsIncrementor = 0;
			while (satellitePositionsFrom2Sec[i].size() <= 1) {
				
				// Create a UTC date and time object for the desired time
				DateTime dt(currentTimeV[0], currentTimeV[1], currentTimeV[2], currentTimeV[3], currentTimeV[4], currentTimeV[5] + secondsIncrementor, currentTimeV[6]);

				// Propagate the satellite's orbit to the desired time
				Eci eci = sgp4.FindPosition(dt);

				float x = eci.Position().x / 100;
				float y = eci.Position().y / 100;
				float z = eci.Position().z / 100;

				satellitePositionsFrom2Sec[i].push_back(createSatellitePos(x, y, z));

				secondsIncrementor = satellitePositionsFrom2Sec[i].size();
			}

			satellitePositions.push_back(interpolate(satellitePositionsFrom2Sec[i][0], satellitePositionsFrom2Sec[i][1], interpolationIncrement));
		}

	}

#undef _SATELLITE_PROPAGATION
#endif
