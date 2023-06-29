#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <sstream>
#include <cctype>
#include <string>
std::string Right_Justification(int total_byte_number, int i){
    int count = 1;
    while(total_byte_number != 0){
        total_byte_number = total_byte_number / 10;
        count += 1;
    }
    if(i == 0){
        count -= 1;
    } else {
        while(i != 0){
            i = i/10;
            count -= 1;
        }
    }
    std::string output_whitespace;
    while(count != 0){
        output_whitespace += ' ';
        count -= 1;
    }
    return output_whitespace;
}

std::string Base_Two_Converter(int input_number){
    // convert to 2's complement
    std::string output_string = "00000000000000000000000000000000";
    int initial_length = 31;
    if(input_number == -2147483648){
        output_string[0] = '1';
        return output_string;
    }
    if(input_number >= 0){
        int remain = 0;
        while(input_number != 0){
            remain = input_number % 2;
            output_string[initial_length] = '0' + remain;
            input_number = input_number / 2;
            initial_length -= 1;
        }
        return output_string;
    } else {
        input_number = -1 * input_number;
        int remain = 0;
        while(input_number != 0){
            remain = input_number % 2;
            output_string[initial_length] = '0' + remain;
            input_number = input_number / 2;
            initial_length -= 1;
        }
        // 1's complement
        for(int i = 31; i >= 0; --i){
            if(output_string[i] == '1'){
                output_string[i] = '0';
            } else {
                output_string[i] = '1';
            }
        }
        // add 1
        int carry = 0;
        initial_length = 31;
        if(output_string[initial_length] == '0'){
            output_string[initial_length] = '1';
        } else {
            output_string[initial_length] = '0';
            carry = 1;
            initial_length -= 1;
            while(carry != 0){
                if(output_string[initial_length] == '0'){
                    output_string[initial_length] = '1';
                    carry = 0;
                } else {
                    output_string[initial_length] = '0';
                    initial_length -= 1;
                }
            }
        }
        return output_string;
    }
}

int main(int argc, char* argv[]) {
    // check the number of command-line arguments
    if(argc != 2){
        std::cerr << "Wrong number of arguments." << std::endl;
        return 1;
    }
    // check whether the file can be opened successfully
    std::string input_filename;
    input_filename = argv[1];
    std::ifstream input_file;
    input_file.open(input_filename);
    if(!input_file.is_open()){
        std::cerr << "Failed to open file." << std::endl;
        return 2;
    }
    std::ifstream input_file_first_run;// use this ifstream variable to get all label values
    input_file_first_run.open(input_filename);
    // preparation for translation
    // create dictionary for registers
    std::map<std::string, std::string> register_dictionary;
    register_dictionary.insert(std::pair<std::string, std::string>("R1", "000"));
    register_dictionary.insert(std::pair<std::string, std::string>("R2", "001"));
    register_dictionary.insert(std::pair<std::string, std::string>("R3", "010"));
    register_dictionary.insert(std::pair<std::string, std::string>("R4", "011"));
    register_dictionary.insert(std::pair<std::string, std::string>("R5", "100"));
    register_dictionary.insert(std::pair<std::string, std::string>("R6", "101"));
    register_dictionary.insert(std::pair<std::string, std::string>("RS", "110"));
    register_dictionary.insert(std::pair<std::string, std::string>("RB", "111"));
    // create dictionary for two operands instruction opcode
    std::map<std::string, std::string> two_operands_instruction_opcode;
    two_operands_instruction_opcode.insert(std::pair<std::string, std::string>("mov", "0000"));
    two_operands_instruction_opcode.insert(std::pair<std::string, std::string>("add", "0001"));
    two_operands_instruction_opcode.insert(std::pair<std::string, std::string>("cmp", "0010"));
    // create dictionary for one operand instruction opcode
    std::map<std::string, std::string> one_operands_instruction_opcode;
    one_operands_instruction_opcode.insert(std::pair<std::string, std::string>("push", "0011"));
    one_operands_instruction_opcode.insert(std::pair<std::string, std::string>("pop", "0100"));
    one_operands_instruction_opcode.insert(std::pair<std::string, std::string>("call", "0101"));
    one_operands_instruction_opcode.insert(std::pair<std::string, std::string>("je", "0110"));
    one_operands_instruction_opcode.insert(std::pair<std::string, std::string>("jge", "0111"));
    one_operands_instruction_opcode.insert(std::pair<std::string, std::string>("jl", "1000"));
    one_operands_instruction_opcode.insert(std::pair<std::string, std::string>("j", "1001"));
    // create dictionary for zero operand instruction opcode
    std::map<std::string, std::string> zero_operands_instruction_opcode;
    zero_operands_instruction_opcode.insert(std::pair<std::string, std::string>("ret", "1010"));
    zero_operands_instruction_opcode.insert(std::pair<std::string, std::string>("nop", "1011"));
    // create dictionary for addressing modes
    std::vector<std::string> addressing_modes_dictionary;
    addressing_modes_dictionary.push_back("00");// immediate mode
    addressing_modes_dictionary.push_back("01");// register mode
    addressing_modes_dictionary.push_back("10");// direct mode
    addressing_modes_dictionary.push_back("11");// indexed mode
    // create dictionary for labels and their addresses
    std::map<std::string, std::string> labels_addresses_dictionary;
    // start translation
    std::string line;// direct read from each line from the file
    std::string line_machine_code;// the conversion result of each line
    int byte_address_record = 0;// record the current byte address
    int previous_byte_address_record = 0;
    std::string byte_address_record_in_string;
    std::vector<std::string> line_content;
    int detect_label_creation = 0;
    std::string model_label_address_test_string = "00000000000000000000000000000000"; // used for label value replacement
    // first run to collect label information
    while(std::getline(input_file_first_run, line)){
        std::stringstream division(line);
        std::string buffer_string;
        while(division >> buffer_string){
            line_content.push_back(buffer_string);
        }
        // whether the line is a two operands instruction
        for(auto const& element : two_operands_instruction_opcode){
            if(line_content[0] == element.first){
                detect_label_creation = 1;
                line_machine_code += element.second;
                // first operand translation
                // immediate mode
                if(line_content[1][0] == '$'){
                    line_machine_code += addressing_modes_dictionary[0];
                    if(std::isdigit(line_content[1][1]) || line_content[1][1] == '-'){
                        std::string some_number_in_string = line_content[1].substr(1);
                        int some_number_in_int = std::stoi(some_number_in_string);
                        std::string some_number_representation = Base_Two_Converter(some_number_in_int);
                        line_machine_code += some_number_representation;
                    } else {
                        std::string label_in_string = line_content[1].substr(1);
                        line_machine_code += model_label_address_test_string;
                    }
                } else if(line_content[1][0] == 'R'){// register mode
                    line_machine_code += addressing_modes_dictionary[1];
                    std::string register_in_string = line_content[1];
                    for(auto const& register_information : register_dictionary){
                        if(register_information.first == register_in_string){
                            line_machine_code += register_information.second;
                            break;
                        }
                    }
                } else if(std::isdigit(line_content[1][0]) || line_content[1][0] == '-'){// indexed mode
                    line_machine_code += addressing_modes_dictionary[3];
                    std::size_t index_of_left_parentheses = line_content[1].find('(');
                    std::string index_number_in_string = line_content[1].substr(0,index_of_left_parentheses);
                    int index_number_in_int = std::stoi(index_number_in_string);
                    std::string index_number_representation = Base_Two_Converter(index_number_in_int);
                    line_machine_code += index_number_representation;
                    std::string indexed_register_in_string = line_content[1].substr(index_of_left_parentheses + 1, 2);
                    for(auto const& register_information : register_dictionary){
                        if(register_information.first == indexed_register_in_string){
                            line_machine_code += register_information.second;
                            break;
                        }
                    }
                } else { // direct mode
                    line_machine_code += addressing_modes_dictionary[2];
                    line_machine_code += model_label_address_test_string;
                }
                // second operand translation
                // immediate mode
                if(line_content[2][0] == '$'){
                    line_machine_code += addressing_modes_dictionary[0];
                    if(std::isdigit(line_content[2][1]) || line_content[2][1] == '-'){
                        std::string some_number_in_string = line_content[2].substr(1);
                        int some_number_in_int = std::stoi(some_number_in_string);
                        std::string some_number_representation = Base_Two_Converter(some_number_in_int);
                        line_machine_code += some_number_representation;
                    } else {
                        std::string label_in_string = line_content[2].substr(1);
                        line_machine_code += model_label_address_test_string;
                    }
                } else if(line_content[2][0] == 'R'){// register mode
                    line_machine_code += addressing_modes_dictionary[1];
                    std::string register_in_string = line_content[2];
                    for(auto const& register_information : register_dictionary){
                        if(register_information.first == register_in_string){
                            line_machine_code += register_information.second;
                            break;
                        }
                    }
                } else if(std::isdigit(line_content[2][0]) || line_content[1][0] == '-'){// indexed mode
                    line_machine_code += addressing_modes_dictionary[3];
                    std::size_t index_of_left_parentheses = line_content[2].find('(');
                    std::string index_number_in_string = line_content[2].substr(0,index_of_left_parentheses);
                    int index_number_in_int = std::stoi(index_number_in_string);
                    std::string index_number_representation = Base_Two_Converter(index_number_in_int);
                    line_machine_code += index_number_representation;
                    std::string indexed_register_in_string = line_content[2].substr(index_of_left_parentheses + 1, 2);
                    for(auto const& register_information : register_dictionary){
                        if(register_information.first == indexed_register_in_string){
                            line_machine_code += register_information.second;
                            break;
                        }
                    }
                } else { // direct mode
                    line_machine_code += addressing_modes_dictionary[2];
                    line_machine_code += model_label_address_test_string;
                }
                break;
            }
        }
        // whether the line is a one operand instruction
        for(auto const& element : one_operands_instruction_opcode) {
            if (line_content[0] == element.first) {
                detect_label_creation = 2;
                line_machine_code += element.second;
                // first operand translation
                // immediate mode
                if (line_content[1][0] == '$') {
                    line_machine_code += addressing_modes_dictionary[0];
                    if (std::isdigit(line_content[1][1]) || line_content[1][1] == '-') {
                        std::string some_number_in_string = line_content[1].substr(1);
                        int some_number_in_int = std::stoi(some_number_in_string);
                        std::string some_number_representation = Base_Two_Converter(some_number_in_int);
                        line_machine_code += some_number_representation;
                    } else {
                        std::string label_in_string = line_content[1].substr(1);
                        line_machine_code += model_label_address_test_string;
                    }
                } else if (line_content[1][0] == 'R') {// register mode
                    line_machine_code += addressing_modes_dictionary[1];
                    std::string register_in_string = line_content[1];
                    for (auto const &register_information: register_dictionary) {
                        if (register_information.first == register_in_string) {
                            line_machine_code += register_information.second;
                            break;
                        }
                    }
                } else if (std::isdigit(line_content[1][0]) || line_content[1][0] == '-') {// indexed mode
                    line_machine_code += addressing_modes_dictionary[3];
                    std::size_t index_of_left_parentheses = line_content[1].find('(');
                    std::string index_number_in_string = line_content[1].substr(0, index_of_left_parentheses);
                    int index_number_in_int = std::stoi(index_number_in_string);
                    std::string index_number_representation = Base_Two_Converter(index_number_in_int);
                    line_machine_code += index_number_representation;
                    std::string indexed_register_in_string = line_content[1].substr(index_of_left_parentheses + 1, 2);
                    for (auto const &register_information: register_dictionary) {
                        if (register_information.first == indexed_register_in_string) {
                            line_machine_code += register_information.second;
                            break;
                        }
                    }
                } else { // direct mode
                    line_machine_code += addressing_modes_dictionary[2];
                    line_machine_code += model_label_address_test_string;
                }
                break;
            }
        }
        // whether the line is a zero operand instruction
        for(auto const& element : zero_operands_instruction_opcode) {
            if (line_content[0] == element.first) {
                detect_label_creation = 3;
                line_machine_code += element.second;
                break;
            }
        }
        // whether the line is a variable declaration
        if(line_content[0] == "var"){
            detect_label_creation = 4;
            int some_unsigned_number = std::stoi(line_content[1]);
            std::string some_unsigned_integer_in_string = Base_Two_Converter(some_unsigned_number);
            line_machine_code += some_unsigned_integer_in_string;
        }
        // whether the line is a label
        if(detect_label_creation == 0){
            for(auto const& label_information : labels_addresses_dictionary){
                if(line_content[0] == label_information.first){
                    std::cerr << "At least one duplicate label found." << std::endl;
                    return 3;
                }
            }
            byte_address_record_in_string = Base_Two_Converter(byte_address_record);
            labels_addresses_dictionary.insert(std::pair<std::string, std::string>(line_content[0], byte_address_record_in_string));
        }
        // adjustment to the bytes
        if(detect_label_creation != 0 && detect_label_creation != 4){
            // we do not do the translation if the current line is a label creation
            // we do not do the adjustment if the current line is a variable declaration since it should be 32 bits
            int current_length = line_machine_code.length();
            int remain_mod8 = current_length % 8;
            int need_to_add = 8 - remain_mod8;
            for(int i =0; i < need_to_add; ++i){
                line_machine_code += '0';
            }
        }
        byte_address_record = previous_byte_address_record + line_machine_code.length() / 8;
        // record the current address index for future loop use
        previous_byte_address_record += line_machine_code.length() / 8;
        // print out current line's machine code
        // clear necessary variable
        line_content.clear();
        line_machine_code.clear();
        detect_label_creation = 0;
    }
    // clear and preparation
    int total_byte_number = byte_address_record - 1;
    line.clear();
    line_machine_code.clear();
    byte_address_record = 0;
    previous_byte_address_record = 0;
    byte_address_record_in_string.clear();
    line_content.clear();
    detect_label_creation = 0;
    // second run to print out final results
    while(std::getline(input_file, line)){
        std::stringstream division(line);
        std::string buffer_string;
        while(division >> buffer_string){
            line_content.push_back(buffer_string);
        }
        // whether the line is a two operands instruction
        for(auto const& element : two_operands_instruction_opcode){
            if(line_content[0] == element.first){
                detect_label_creation = 1;
                line_machine_code += element.second;
                // first operand translation
                // immediate mode
                if(line_content[1][0] == '$'){
                    line_machine_code += addressing_modes_dictionary[0];
                    if(std::isdigit(line_content[1][1]) || line_content[1][1] == '-'){
                        std::string some_number_in_string = line_content[1].substr(1);
                        int some_number_in_int = std::stoi(some_number_in_string);
                        std::string some_number_representation = Base_Two_Converter(some_number_in_int);
                        line_machine_code += some_number_representation;
                    } else {
                        std::string label_in_string = line_content[1].substr(1);
                        for(auto const& label_information : labels_addresses_dictionary){
                            if(label_information.first == label_in_string){
                                line_machine_code += label_information.second;
                                break;
                            }
                        }
                    }
                } else if(line_content[1][0] == 'R'){// register mode
                    line_machine_code += addressing_modes_dictionary[1];
                    std::string register_in_string = line_content[1];
                    for(auto const& register_information : register_dictionary){
                        if(register_information.first == register_in_string){
                            line_machine_code += register_information.second;
                            break;
                        }
                    }
                } else if(std::isdigit(line_content[1][0]) || line_content[1][0] == '-'){// indexed mode
                    line_machine_code += addressing_modes_dictionary[3];
                    std::size_t index_of_left_parentheses = line_content[1].find('(');
                    std::string index_number_in_string = line_content[1].substr(0,index_of_left_parentheses);
                    int index_number_in_int = std::stoi(index_number_in_string);
                    std::string index_number_representation = Base_Two_Converter(index_number_in_int);
                    line_machine_code += index_number_representation;
                    std::string indexed_register_in_string = line_content[1].substr(index_of_left_parentheses + 1, 2);
                    for(auto const& register_information : register_dictionary){
                        if(register_information.first == indexed_register_in_string){
                            line_machine_code += register_information.second;
                            break;
                        }
                    }
                } else { // direct mode
                    line_machine_code += addressing_modes_dictionary[2];
                    for(auto const& label_information : labels_addresses_dictionary){
                        if(label_information.first == line_content[1]){
                            line_machine_code += label_information.second;
                            break;
                        }
                    }
                }
                // second operand translation
                // immediate mode
                if(line_content[2][0] == '$'){
                    line_machine_code += addressing_modes_dictionary[0];
                    if(std::isdigit(line_content[2][1]) || line_content[2][1] == '-'){
                        std::string some_number_in_string = line_content[2].substr(1);
                        int some_number_in_int = std::stoi(some_number_in_string);
                        std::string some_number_representation = Base_Two_Converter(some_number_in_int);
                        line_machine_code += some_number_representation;
                    } else {
                        std::string label_in_string = line_content[2].substr(1);
                        for(auto const& label_information : labels_addresses_dictionary){
                            if(label_information.first == label_in_string){
                                line_machine_code += label_information.second;
                                break;
                            }
                        }
                    }
                } else if(line_content[2][0] == 'R'){// register mode
                    line_machine_code += addressing_modes_dictionary[1];
                    std::string register_in_string = line_content[2];
                    for(auto const& register_information : register_dictionary){
                        if(register_information.first == register_in_string){
                            line_machine_code += register_information.second;
                            break;
                        }
                    }
                } else if(std::isdigit(line_content[2][0]) || line_content[1][0] == '-'){// indexed mode
                    line_machine_code += addressing_modes_dictionary[3];
                    std::size_t index_of_left_parentheses = line_content[2].find('(');
                    std::string index_number_in_string = line_content[2].substr(0,index_of_left_parentheses);
                    int index_number_in_int = std::stoi(index_number_in_string);
                    std::string index_number_representation = Base_Two_Converter(index_number_in_int);
                    line_machine_code += index_number_representation;
                    std::string indexed_register_in_string = line_content[2].substr(index_of_left_parentheses + 1, 2);
                    for(auto const& register_information : register_dictionary){
                        if(register_information.first == indexed_register_in_string){
                            line_machine_code += register_information.second;
                            break;
                        }
                    }
                } else { // direct mode
                    line_machine_code += addressing_modes_dictionary[2];
                    for(auto const& label_information : labels_addresses_dictionary){
                        if(label_information.first == line_content[2]){
                            line_machine_code += label_information.second;
                            break;
                        }
                    }
                }
                break;
            }
        }
        // whether the line is a one operand instruction
        for(auto const& element : one_operands_instruction_opcode) {
            if (line_content[0] == element.first) {
                detect_label_creation = 2;
                line_machine_code += element.second;
                // first operand translation
                // immediate mode
                if (line_content[1][0] == '$') {
                    line_machine_code += addressing_modes_dictionary[0];
                    if (std::isdigit(line_content[1][1]) || line_content[1][1] == '-') {
                        std::string some_number_in_string = line_content[1].substr(1);
                        int some_number_in_int = std::stoi(some_number_in_string);
                        std::string some_number_representation = Base_Two_Converter(some_number_in_int);
                        line_machine_code += some_number_representation;
                    } else {
                        std::string label_in_string = line_content[1].substr(1);
                        for (auto const &label_information: labels_addresses_dictionary) {
                            if (label_information.first == label_in_string) {
                                line_machine_code += label_information.second;
                                break;
                            }
                        }
                    }
                } else if (line_content[1][0] == 'R') {// register mode
                    line_machine_code += addressing_modes_dictionary[1];
                    std::string register_in_string = line_content[1];
                    for (auto const &register_information: register_dictionary) {
                        if (register_information.first == register_in_string) {
                            line_machine_code += register_information.second;
                            break;
                        }
                    }
                } else if (std::isdigit(line_content[1][0]) || line_content[1][0] == '-') {// indexed mode
                    line_machine_code += addressing_modes_dictionary[3];
                    std::size_t index_of_left_parentheses = line_content[1].find('(');
                    std::string index_number_in_string = line_content[1].substr(0, index_of_left_parentheses);
                    int index_number_in_int = std::stoi(index_number_in_string);
                    std::string index_number_representation = Base_Two_Converter(index_number_in_int);
                    line_machine_code += index_number_representation;
                    std::string indexed_register_in_string = line_content[1].substr(index_of_left_parentheses + 1, 2);
                    for (auto const &register_information: register_dictionary) {
                        if (register_information.first == indexed_register_in_string) {
                            line_machine_code += register_information.second;
                            break;
                        }
                    }
                } else { // direct mode
                    line_machine_code += addressing_modes_dictionary[2];
                    for (auto const &label_information: labels_addresses_dictionary) {
                        if (label_information.first == line_content[1]) {
                            line_machine_code += label_information.second;
                            break;
                        }
                    }
                }
                break;
            }
        }
        // whether the line is a zero operand instruction
        for(auto const& element : zero_operands_instruction_opcode) {
            if (line_content[0] == element.first) {
                detect_label_creation = 3;
                line_machine_code += element.second;
                break;
            }
        }
        // whether the line is a variable declaration
        if(line_content[0] == "var"){
            detect_label_creation = 4;
            int some_unsigned_number = std::stoi(line_content[1]);
            std::string some_unsigned_integer_in_string = Base_Two_Converter(some_unsigned_number);
            line_machine_code += some_unsigned_integer_in_string;
        }
        // adjustment to the bytes
        if(detect_label_creation != 0 && detect_label_creation != 4){
            // we do not do the translation if the current line is a label creation
            // we do not do the adjustment if the current line is a variable declaration since it should be 32 bits
            int current_length = line_machine_code.length();
            int remain_mod8 = current_length % 8;
            int need_to_add = 8 - remain_mod8;
            for(int i =0; i < need_to_add; ++i){
                line_machine_code += '0';
            }
        }
        // print out current line's machine code
        if(detect_label_creation != 0){// we do not print anything for label creation
            byte_address_record = previous_byte_address_record + line_machine_code.length() / 8;
            int index_for_print_out = 0;
            for(int i = previous_byte_address_record; i < byte_address_record; ++i){
                std::string print_out_each_byte = line_machine_code.substr(index_for_print_out, 8);
                // right justification
                std::cout << "Byte" << Right_Justification(total_byte_number, i) << i << ": " << print_out_each_byte << std::endl;
                // update index for print out
                index_for_print_out += 8;
            }
            // record the current address index for future loop use
            previous_byte_address_record += line_machine_code.length() / 8;
        }
        // clear necessary variable
        line_content.clear();
        line_machine_code.clear();
        detect_label_creation = 0;
    }
    return 0;
}
