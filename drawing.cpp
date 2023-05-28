#ifdef _DRAWING
		
	/*glPushMatrix();
		glTranslatef(0,1,0);
		drawModel("sky");
	glPopMatrix();*/

	glPushMatrix();
		glTranslatef(8,2,-152);
		glRotatef(160,0,1,0);
		drawModel ("`");
	glPopMatrix();

	Sphere sphere1(1.0f, 36, 18, false, 2);
	Sphere sphere2(1.0f, 36, 18, true, 2);

	float lineColor[] = { 0.2f, 0.2f, 0.2f, 1 };

	glPushMatrix();
		glutSolidSphere(63.71f, 50, 50);
	glPopMatrix();

	for (SatellitePos satellitePos : satellitePositions) {
		glPushMatrix();
			glTranslatef(satellitePos.x, satellitePos.y, satellitePos.z);
			glutSolidSphere(10.0, 50, 50);
		glPopMatrix();
	}
	

#undef _DRAWING
#endif
