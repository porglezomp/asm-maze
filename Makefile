APP=ASMaze
SRC=fbhelper.c framebuffer.s maze.s

$(APP): $(SRC)
	gcc -o $(APP) $(SRC) -g

clean:
	rm -f $(APP)
