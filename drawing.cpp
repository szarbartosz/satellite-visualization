#ifdef _DRAWING
		
	glPushMatrix();
		glTranslatef(0,1,0);
		drawModel("sky");
	glPopMatrix();

	glPushMatrix();
		glTranslatef(8,2,-152);
		glRotatef(160,0,1,0);
		drawModel ("`");
	glPopMatrix();

	Sphere sphere1(1.0f, 36, 18, false, 2);
	Sphere sphere2(1.0f, 36, 18, true, 2);

	float lineColor[] = { 0.2f, 0.2f, 0.2f, 1 };

	sphere1.drawWithLines(lineColor);


    glPushMatrix();
    glTranslatef(-2.5f, 0, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    sphere1.drawWithLines(lineColor);
    sphere1.drawLines(lineColor);
    glPopMatrix();


    glPushMatrix();
    glBindTexture(GL_TEXTURE_2D, 0);
    sphere2.drawWithLines(lineColor);
    glPopMatrix();

#undef _DRAWING
#endif
