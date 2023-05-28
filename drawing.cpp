#ifdef _DRAWING
	// glPushMatrix();
	// 	glTranslatef(-125.0f, 0, 0);
	// 	drawModel("baloon");
	// glPopMatrix();

	// glPushMatrix();
	// 	glTranslatef(125.0f, 0, 0);
	// 	drawModel("satellite");
	// glPopMatrix();

	for (SatellitePos satellitePos : satellitePositions) {
		glPushMatrix();
			glTranslatef(satellitePos.x, satellitePos.y, satellitePos.z);
			glScalef(0.4f, 0.4f, 0.4f);
			drawModel("satellite");
		glPopMatrix();
	}

	glPushMatrix();
		glBindTexture(GL_TEXTURE_2D, earthTexId);
		glScalef(63.71f, 63.71f, 63.71f);
		earthSphere.draw();
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, 0);
	showInfo();

#undef _DRAWING
#endif
