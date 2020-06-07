#
# Date:      2013/06/10 15:21
# Author:    Jan Faigl
#

uniq = $(if $1,$(firstword $1) $(call uniq,$(filter-out $(firstword $1),$1)))

CXXFLAGS:=$(call uniq,$(CXXFLAGS))

bin: $(TARGET)

$(OBJS): %.o: %.cc
	$(CXX) -c $< $(CXXFLAGS) $(CPPFLAGS) -o $@

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o $@

clean:
	$(RM) $(OBJS) $(TARGET)

x: clean bin


