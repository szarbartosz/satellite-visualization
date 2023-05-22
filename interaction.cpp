#ifdef _INTERACTION

void OnMouseClick (int button, int state, int x, int y)
{
	DefaultOnMouseClick (button, state, x, y);
}

void OnMousePane (int x, int y)
{
	DefaultOnMousePane (x, y);
}

void OnKeyPress (GLubyte key, int x, int y)
{
	DefaultOnKeyPress (key, x, y);

	switch (key) 
	{
		case 'k':
			exit(1);
			break;
	}
}

void OnSpecialKeyPress (GLint key, int x, int y)
{	
	switch (key) 
	{
		case GLUT_KEY_LEFT:

			break;
		case GLUT_KEY_RIGHT:

			break;
		case GLUT_KEY_UP:

			break;
		case GLUT_KEY_DOWN:

			break;

	}
}

#undef _INTERACTION
#endif
