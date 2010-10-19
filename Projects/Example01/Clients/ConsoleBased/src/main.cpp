#include <msgpack.hpp>
int main(int argc,char **argv)
{
	msgpack::sbuffer buf;
	std::string xx="packme";
	msgpack::pack(buf, xx);

	return 0;
}
