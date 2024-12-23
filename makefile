
########################### VARIABLES ############################

TARGET      = test
COMPILER    = gcc
C_RELEASE   = -c -O3 -Wall
C_DEBUG     = -ggdb -c -Wall
C_FLAGS     = $(C_RELEASE)
#C_FLAGS     = $(C_DEBUG)
LD_FLAGS    =
SOURCES     =
OBJECTS     =
OBJECTS_ADD =

############################ FOLDERS #############################



##################################################################

C_FLAGS +=

##################################################################

LD_FLAGS +=

##################################################################

SOURCES += main.c

##################################################################

OBJECTS += $(SOURCES:.c=.o)

##################################################################

all: $(SOURCES) $(TARGET)

##################################################################

$(TARGET): $(OBJECTS) $(OBJECTS_ADD)
	$(COMPILER) $(OBJECTS) $(OBJECTS_ADD) -o $(TARGET) $(LD_FLAGS)

.c.o:
	$(COMPILER) $(C_FLAGS) $< -o $@

##################################################################

rmo:
	rm $(OBJECTS)

clean:
	rm $(OBJECTS) $(TARGET)
