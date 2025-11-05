
#include <iostream>
#include "gbmodal_interface.hpp"

static void help(std::string & program_name)
{
	std::cout << std::endl;
	std::cout << "This sample demostrates a tetrahedron model." << std::endl;
	std::cout << "Call" << std::endl;
	std::cout << program_name << "box_case_1.inp" << std::endl;
	std::cout << std::endl;

}

int main(int argc, char* argv[]) 
{
    std::string program_name = argv[0];
    if (argc ==1) {
      help(program_name);
    }

    GBModalInterface gbmodal;
    gbmodal.read_gearbox_modal_data(argv[1]);
    gbmodal.write_to_vtu(argv[2]);

    return 0;
}
