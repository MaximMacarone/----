#include <iostream>
#include <vector>
#include <bitset>
#include <cmath>
#include <string>

//symbol structure for arithmetic coding based on integers
struct symbol {
	unsigned value; // symbol value
	unsigned freq; // symbol frequency
	unsigned int low; // symbol low bound
	unsigned int high; // symbol high bound
};

//symbol structure for simple arithmetic coding based on floating point numbers
struct symbol_simple {
	unsigned value; // symbol value
	double freq; // symbol frequency
	double low; // symbol low bound
	double high; // symbol high bound
};

unsigned calculate_encoded_size(const std::vector<symbol>& symbols) {
	unsigned total_frequency = 0; // total frequency of all symbols
	for (int i = 0; i < symbols.size(); i++) { // calculate total frequency
		total_frequency += symbols[i].freq;
	}

	std::vector<double> probabilities; // symbol probabilities
	for (int i = 0; i < symbols.size(); i++) { // calculate probabilities
		probabilities.push_back((double)symbols[i].freq / total_frequency);
	}

	std::vector<double> cumulative_probabilities; // cumulative probabilities
	double cumulative_probability = 0;
	for (int i = 0; i < probabilities.size(); i++) { // calculate cumulative probabilities

		cumulative_probability += probabilities[i];
		cumulative_probabilities.push_back(cumulative_probability);

	}

	double size = 0; // encoded message size
	// calculate expected bits for each symbol
	for (size_t i = 0; i < symbols.size(); i++) {
		if (probabilities[i] != 0) {
			double lowerBound = cumulative_probabilities[i] - probabilities[i];
			double upperBound = cumulative_probabilities[i];
			double rangeSize = upperBound - lowerBound;

			double bitsPerSymbol = -log2(rangeSize);
			double expectedBitsForSymbol = symbols[i].freq * bitsPerSymbol;

			size += expectedBitsForSymbol;
		}

	}

	return int(ceil(size));
}

//Encoding based on integer numbers
void acoder(unsigned* input, int inSize, unsigned* encoded, int* encBitSize, std::vector<symbol>& symbols);
void decoder(unsigned* encoded, int encBitSize, unsigned* decoded, int* decSize, std::vector<symbol> symbols);

//Encoding based on floating point numbers
void acoder_simple(unsigned* input, int inSize, long double* encoded, int* encBitSize, std::vector<symbol_simple>& symbols);
void decoder_simple(long double* encoded, int encBitSize, unsigned* decoded, int* decSize, std::vector<symbol_simple> symbols);

//input array size
const int inSize = 300;

int main() {

	//unsigned encoded = 0; // encoded message
	//unsigned int array of inSize size with random elements between 0 and 255
	unsigned input[inSize];
	for (int i = 0; i < inSize; i++) {
		input[i] = rand() % 250;
	}
	
	int encBitSize = 0; // encoded message bit size
	unsigned decoded[inSize];
	int decSize = 0;

	//symbol frequencies in input message
	std::vector<unsigned> freq(257, 0); // 256 + 1 for EOF
	for (int i = 0; i < inSize; i++) {
		freq[input[i]]++;
	}
	freq[256] = 1; // EOF

	std::cout << "Arithmetic coding" << std::endl << std::endl;
	std::cout << "1. Floating point number arithmetic coding" << std::endl;
	std::cout << "2. Integer arithmetic coding" << std::endl << std::endl;

	std::cout << "Choose arithmetic coding type: ";
	int choice;
	std::cin >> choice;
	std::cout << std::endl;

	//switch between integer and floating point arithmetic coding
	switch (choice) {
	case 1: {

		// cummulative frequencies for symbols for simple arithmetic coding
		std::vector<symbol_simple> symbols_simple;
		unsigned short int low = 0;
		for (int i = 0; i < 257; i++) {
			symbol_simple s;
			s.value = i;
			s.freq = freq[i];
			s.low = low;
			s.high = low + freq[i];
			symbols_simple.push_back(s);
			low += freq[i];
		}

		system("cls");

		std::cout << "Floating point number arithmetic coding" << std::endl << std::endl;

		long double encoded_simple = 0;
		acoder_simple(input, inSize, &encoded_simple, &encBitSize, symbols_simple);
		std::cout << "Encoded: " << encoded_simple << std::endl;


		decoder_simple(&encoded_simple, encBitSize, decoded, &decSize, symbols_simple);
		std::cout << "Decoded: ";
		for (int i = 0; i < decSize; i++) {
			std::cout << decoded[i] << " ";
		}
		std::cout << std::endl;

		break;
		}
	case 2: {
		//cumulative frequencies for symbols for arithmetic coding
		std::vector<symbol> symbols;
		unsigned short int low = 0;
		for (int i = 0; i < 257; i++) {
			symbol s;
			s.value = i;
			s.freq = freq[i];
			s.low = low;
			s.high = low + freq[i];
			symbols.push_back(s);
			low += freq[i];
		}

		unsigned encoded_size = calculate_encoded_size(symbols);
		//encoded message array
		unsigned int* encoded = new unsigned int[int(ceil(double(encoded_size) / 32)) + 1];
		encoded[int(ceil(double(encoded_size) / 32))] = 0xFFFFFFFF;
		

		system("cls");

		std::cout << "Integer arithmetic coding" << std::endl << std::endl;

		acoder(input, inSize, encoded, &encBitSize, symbols);

		std::cout << "Encoded: ";
		for (int i = 0; i < ceil(double(encoded_size) / 32 + 1); i++) {
			std::cout << encoded[i] << " ";
		}
		std::cout << "\nBinary encoded: ";
		for (int i = 0; i < ceil(double(encoded_size) / 32 + 1); i++) {
			std::cout << std::bitset<32>(encoded[i]) << " ";
		}
		

		std::cout << "\nEncoded bit size: " << encBitSize << std::endl;
		system("pause");


		decSize = 0;
		decoder(encoded, encBitSize, decoded, &decSize, symbols);
		std::cout << "\nDecoded: ";
		for (int i = 0; i < decSize; i++) {
			std::cout << decoded[i] << " ";
		}
		std::cout << std::endl;

		}
	}

	return 0;
}


void acoder(unsigned* input, int inSize, unsigned* encoded, int* encBitSize, std::vector<symbol>& symbols) {
	unsigned short int low = 0;			//low bound of interval
	unsigned short int high = 0xFFFF;	//high bound of interval
	int underflow_counter = 0;			//underflow counter
	std::string encoded_str = "";		//string type encoded message
	inSize++; // add EOF
	std::cout << "Message: ";
	for (int i = 0; i < inSize - 1; i++) {
		std::cout << input[i] << " ";
	}
	std::cout << "EOF" << std::endl;

	std::cout << "low:  " << std::bitset<16>(low) << std::endl;
	std::cout << "high: " << std::bitset<16>(high) << std::endl;

	//iterating through input message
	for (int i = 0; i < inSize; i++) {
		unsigned int range = high - low + 1;
		std::cout << "Iteration: " << i << std::endl << std::endl;

		
		if (i < inSize - 1) { //if not EOF
			std::cout << "symbol: " << input[i] << std::endl;
			std::cout << "cum_freq low:  " << symbols[input[i]].low << std::endl;
			std::cout << "cum_freq high: " << symbols[input[i]].high << std::endl;
			//zoom in to symbol's interval
			high = low + (range * symbols[input[i]].high) / inSize;
			low = low + (range * symbols[input[i]].low) / inSize;

			std::cout << "new low:  " << std::bitset<16>(low) << std::endl;
			std::cout << "new high: " << std::bitset<16>(high) << std::endl;
		}
		else { //if EOF
			std::cout << "symbol: " << symbols.back().value << std::endl;
			std::cout << "cum_freq low:  " << symbols.back().low << std::endl;
			std::cout << "cum_freq high: " << symbols.back().high << std::endl;
			high = low + (range * symbols.back().high) / inSize;
			low = low + (range * symbols.back().low) / inSize;
			std::cout << "new low:  " << std::bitset<16>(low) << std::endl;
			std::cout << "new high: " << std::bitset<16>(high) << std::endl;
		}

		while (true) {
			//if MSB of low and high are equal
			if ((high & 0x8000) == (low & 0x8000)) {
				//shift out MSB of low and high
				encoded_str += std::to_string(low >> 15);
				*encBitSize += 1;
				//shift out MSB of underflow_counter
				for (int i = 0; i < underflow_counter; i++) {
					encoded_str += std::to_string(!(low >> 15));
					*encBitSize += 1;
				}
				underflow_counter = 0;

				//shift out MSB of low and high
				low = (low << 1) & 0xFFFF;
				high = ((high << 1) & 0xFFFF) | 1;
			}
			
			//else if second bit of low is 1 and second bit of high is 0
			else if ((low & 0x4000) && !(high & 0x4000)) {
				//splice out second bit of low and high
				underflow_counter++;
				bool first_bit = low & 0x8000;
				low = (low & 0x3FFF) << 1;
				if (first_bit) {
					low = low | 0x8000;
				}
				first_bit = high & 0x8000;
				high = ((high | 0x4000) << 1) | 1;
				if (first_bit) {
					high = high | 0x8000;
				}
				
			}
			else {
				std::cout << "\nAfter shifts: " << std::endl;
				std::cout << "low:  " << std::bitset<16>(low) << std::endl;
				std::cout << "high: " << std::bitset<16>(high) << std::endl;
				std::cout << "Encoded string: " << encoded_str << std::endl << std::endl;
				break;
			}
		}
	}



	//add 01 to the end of the encoded message since that way it represents the shortest representation of encoded message
	encoded_str += std::to_string(0);
	encoded_str += std::to_string(1);
	*encoded = (*encoded << 2) | 1;
	*encBitSize += 2;

	//take each 32 bits of encoded string and convert it to unsigned int and store it in encoded
	int index = 0;
	for (int i = 0; i < encoded_str.length(); i += 32) {
		std::string temp = encoded_str.substr(i, 32);
		if (temp.length() < 32) {
			temp += std::string(32 - temp.length(), '1');
		}
		encoded[index] = (std::stoul(temp, nullptr, 2));
		index++;
	}

}

void decoder(unsigned* encoded_msg, int encBitSize, unsigned* decoded, int* decSize, std::vector<symbol> symbols) {
	std::cout << "\n\nDecoding\n";
	unsigned short int low = 0;
	unsigned short int high = 0xFFFF;
	unsigned int total = 0;	//total cumulative frequency
	int j = 0;	//index of decoded symbol
	int bit_counter = 0;
	int arr_elem = 0;
	
	unsigned short int encoded;
	if (encBitSize > 32) {
		//encoded = first 16 bits of encoded bitstream
		encoded = *encoded_msg >> 16;
		//shift encoded_msg left by 16 bits and shift in next bits from encoded bitstream
		for (int i = 0; i < 16; i++) {
			*encoded_msg = (*encoded_msg << 1);
			bit_counter = (bit_counter + 1) % 31;
			if (bit_counter == 0) {
				arr_elem++;
			}
		}
	}
	else {
		//if encoded message is less or equal to 16 bits
		encoded = *encoded_msg >> 16;
		for (int i = 0; i < 16; i++) {
			*encoded_msg = (*encoded_msg << 1) | 1;
		}
	}
	std::cout << "First 16 bits of encoded message: " << std::bitset<16>(encoded) << std::endl;

	std::cout << "Encoded message after moving left by 16 bits: " << std::bitset<32>(*encoded_msg) << std::endl;
	system("pause");

	//calculate total cumulative frequency
	for (int i = 0; i < symbols.size(); i++) {
		total += symbols[i].freq;
	}

	while (true) {
		std::cout << "\n\nIteration: " << j << std::endl;
		std::cout << "low:  " << std::bitset<16>(low) << std::endl;
		std::cout << "high: " << std::bitset<16>(high) << std::endl;
		unsigned int range = high - low + 1;
		unsigned short int scaled_sym = ((encoded - low + 1) * total - 1) / range;
		std::cout << "scaled_sym: " << std::bitset<16>(scaled_sym) << std::endl;

		int i = 0;
		while (scaled_sym >= (symbols[i].high)) {	// Search symbol such that scaled_sym is in its range
			i++;
		}

		//if EOF symbol is found, break
		if (symbols[i].value == 256) {
			std::cout << "EOF symbol found" << std::endl;
			break;
		}
		
		decoded[j] = symbols[i].value;	//Add decoded symbol to decoded message
		j++; //increment index of decoded message
		std::cout << "Decoded symbol: " << symbols[i].value << std::endl;
		*decSize += 1; //increment size of decoded message

		//update low and high for next iteration
		high = low + (range * symbols[i].high) / total;
		low = low + (range * symbols[i].low) / total;
		std::cout << "new low:  " << std::bitset<16>(low) << std::endl;
		std::cout << "new high: " << std::bitset<16>(high) << std::endl;

		while (true) {
			//if MSB of low and high are equal
			if ((high & 0x8000) == (low & 0x8000)) {
				//shift out MSB of low and high
				low = (low << 1);
				high = (high << 1) | 1;

				std::cout << "Encoded_msg: " << std::bitset<32>(*encoded_msg) << std::endl;
				if (encBitSize <= 32 || arr_elem == 0) {
					std::cout << "bit_counter = " << bit_counter << " arr_elem = " << arr_elem << std::endl;
					std::cout << "Next bit of encoded message: " << std::bitset<16>((*encoded_msg >> 31) & 1) << std::endl;
					encoded = (encoded << 1) | ((*encoded_msg >> 31) & 1);	//shift in next bit from encoded bitstream
					*encoded_msg = *encoded_msg << 1;
					bit_counter = (bit_counter + 1) % 32;
					if (bit_counter == 0) {
						arr_elem++;
					}
				}
				else {
					std::cout << "bit_counter = " << bit_counter << " arr_elem = " << arr_elem << std::endl;
					std::cout << "Next bit of encoded message: " << std::bitset<32>((*(encoded_msg + arr_elem) >> 31 - bit_counter) & 1) << std::endl;
					bool next_bit = ((*(encoded_msg + arr_elem) >> 31 - bit_counter) & 1);
					encoded = (encoded << 1) | next_bit;
					*encoded_msg = *encoded_msg << 1 | ((*(encoded_msg + arr_elem) >> 31 - bit_counter) & 1); 
					bit_counter = (bit_counter + 1) % 32;
					if (bit_counter == 0) {
						arr_elem++;
					}
				}

				std::cout << "Encoded_msg after shift: " << std::bitset<32>(*encoded_msg) << std::endl;
			}
			//else if second bit of low is 1 and second bit of high is 0
			else if ((low & 0x4000) && !(high & 0x4000)) {
				//splice out second bit of low and high
				bool first_bit = low & 0x8000;
				low = (low & 0x3FFF) << 1;
				if (first_bit) {
					low = low | 0x8000;
				}
				first_bit = high & 0x8000;
				high = ((high | 0x4000) << 1) | 1;
				if (first_bit) {
					high = high | 0x8000;
				}
				std::cout << "Encoded: " << std::bitset<16>(encoded) << std::endl;
				first_bit = encoded & 0x8000;
				encoded = (encoded & 0x3FFF) << 1;
				
				if (encBitSize <= 32 || arr_elem == 0) {
					std::cout << "bit_counter = " << bit_counter << " arr_elem = " << arr_elem << std::endl;
					std::cout << "Next bit of encoded message: " << std::bitset<16>((*encoded_msg >> 31) & 1) << std::endl;
					encoded = encoded | ((*encoded_msg >> 31) & 1);	//shift in next bit from encoded bitstream
					*encoded_msg = *encoded_msg << 1;
					bit_counter = (bit_counter + 1) % 32;
					if (bit_counter == 0) {
						arr_elem++;
					}
				}
				else {
					std::cout << "bit_counter = " << bit_counter << " arr_elem = " << arr_elem << std::endl;
					std::cout << "Next bit of encoded message: " << std::bitset<32>((*(encoded_msg + arr_elem) >> 31 - bit_counter) & 1) << std::endl;
					bool next_bit = ((*(encoded_msg + arr_elem) >> 31 - bit_counter) & 1);
					encoded = encoded | next_bit;
					*encoded_msg = *encoded_msg << 1 | ((*(encoded_msg + arr_elem) >> 31 - bit_counter) & 1);
					bit_counter = (bit_counter + 1) % 32;
					if (bit_counter == 0) {
						arr_elem++;
					}
				}
				if (first_bit) {
					encoded = encoded | 0x8000;
				}
				std::cout << "Encoded: " << std::bitset<16>(encoded) << std::endl;
			}
			else {
				std::cout << "\nAfter shifts: " << std::endl;
				std::cout << "low:  " << std::bitset<16>(low) << std::endl;
				std::cout << "high: " << std::bitset<16>(high) << std::endl;
				std::cout << "Encoded: " << std::bitset<16>(encoded) << std::endl << std::endl;

				break;
			}
		}
	}
}

void acoder_simple(unsigned* input, int inSize, long double* encoded, int* encBitSize, std::vector<symbol_simple>& symbols) {
	long double low = 0;	//low bound of range
	long double high = 1;	//high bound of range
	long double range = 0;

	//calculate probabilities
	for (auto& elem : symbols) {
		elem.freq /= inSize + 1;
		elem.low /= inSize + 1;
		elem.high /= inSize + 1;
	}

	//iterating through each symbol in input exluding EOF symbol
	for (int i = 0; i < inSize; i++) {
		range = high - low;
		high = low + (range * symbols[input[i]].high); //zoom in 
		low = low + (range * symbols[input[i]].low); //zoom in
	}
	//add EOF symbol
	range = high - low;
	high = low + (range * symbols.back().high);
	low = low + (range * symbols.back().low);

	*encoded = low; //set encoded value to low bound of range
}

void decoder_simple(long double* encoded, int encBitSize, unsigned* decoded, int* decSize, std::vector<symbol_simple> symbols) {
	long double low = 0;	//low bound of range
	long double high = 1;	//high bound of range
	long double range = 0;
	
	//iterating until the decoding of EOF symbol
	while (true) {
		range = high - low;
		long double scaled_sym = ((*encoded - low) / range); //scaled range 
		int i = 0;
		//choose symbol that corresponds to scaled range
		while (scaled_sym >= (symbols[i].high)) {
			i++;
		}
		//if EOF
		if (symbols[i].value == 256) {
			break;
		}

		decoded[*decSize] = symbols[i].value; // add decoded symbol to decoded array
		*decSize += 1;	//increment decoded array size

		high = low + (range * symbols[i].high); //zoom in on symbol range
		low = low + (range * symbols[i].low); //zoom in on symbol range

	}
}