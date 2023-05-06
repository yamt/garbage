# Note: make distinguishes tabs and spaces.

# = and += are documented in:
# https://www.gnu.org/software/make/manual/html_node/Setting.html
# https://www.gnu.org/software/make/manual/html_node/Appending.html
SRCS = main.m
SRCS += app_delegate.m
SRCS += view.m
SRCS += window.m
BIN = test

# This sets OBJS variable by replacing suffixes in each names in SRCS.
# eg. if SRCS is "a.m b.m", OBJS will be "a.o b.o".
# cf. https://www.gnu.org/software/make/manual/html_node/Substitution-Refs.html
OBJS = $(SRCS:.m=.o)

# A Makefile consists of rules.
# A rule describes how to generate a target, which is usually a file.
# cf.
# https://www.gnu.org/software/make/manual/html_node/Rule-Example.html

# this rule is just for testing.
.PHONY: run
run: $(BIN)
	./$(BIN)

# This rule links object files (*.o files) together to generate
# the executable. (test)
# Note: CC is "cc" by default.
# cf. https://www.gnu.org/software/make/manual/html_node/Implicit-Variables.html
$(BIN): $(OBJS)
	$(CC) -framework Foundation -framework Cocoa -o $@ $(OBJS)

# This rule generates object files (*.o) from source files. (*.m)
# eg. main.m -> main.o
# Note: $@ and $< are documented in https://www.gnu.org/software/make/manual/html_node/Automatic-Variables.html
.m.o:
	$(CC) -c -Wall -o $@ $<

# This rule deletes all generated files.
# Note: .PHONY is a special target.
# cf. https://www.gnu.org/software/make/manual/html_node/Phony-Targets.html
.PHONY: clean
clean:
	rm -f $(BIN)
	rm -f $(OBJS)
