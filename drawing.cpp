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

#undef _DRAWING
#endif
