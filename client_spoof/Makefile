LIBD = ./lib
OBJD = ./obj
HEAD = ./head
OUT = ./output.out
DEPS =
FLAGS = -lncurses

DEPSO = $(addprefix $(OBJD)/,$(addsuffix .o,$(DEPS)))

all : $(OUT)


$(OBJD)/%.o : $(LIBD)/%.cpp
	g++ -c $ $< -o $@ -I $(HEAD)/ $(FLAGS)

$(OBJD)/%.o : $(LIBD)/%.hpp
	g++ -c $ $< -o $@ $(FLAGS)

$(OUT) : $(DEPSO) $(HEAD)
	g++ -o $@ client.cpp $(DEPSO) -I $(HEAD)/ $(FLAGS)
