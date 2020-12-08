#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <stack>
#include <chrono>

enum com : uint_fast8_t {
	IncDptr = '>',
	DecDptr = '<',
	IncCell = '+',
	DecCell = '-',
	OutCell = '.',
	InpCell = ',',
	JumpFwd = '[',
	JumpBwd = ']',
};

struct ins {
	enum com com;
	ssize_t data;
};

std::vector<char> read_file(const std::string &filename) {
	auto content = std::vector<char>();
	auto buf = std::array<char, 4096>();
	auto fs = std::ifstream(filename);

	while (not fs.eof()) {
		auto count = fs.read(buf.data(), sizeof(buf)).gcount();
		content.insert(content.end(), buf.begin(), buf.begin() + count);
	}

	return content;
}

std::vector<struct ins> translate(const std::vector<char> &source) {
	auto program = std::vector<struct ins>();

	for (const auto ch : source) {
		switch (ch) {
			case '>': program.push_back({IncDptr, 1});
				break;
			case '<': program.push_back({DecDptr, -1});
				break;
			case '+': program.push_back({IncCell, 1});
				break;
			case '-': program.push_back({DecCell, -1});
				break;
			case '.': program.push_back({OutCell, 0});
				break;
			case ',': program.push_back({InpCell, 0});
				break;
			case '[': program.push_back({JumpFwd, 0});
				break;
			case ']': program.push_back({JumpBwd, 0});
				break;
			default: break;
		}
	}

	return program;
}

void annotate_forward(std::vector<struct ins> &program) {
	auto stack = std::stack<ssize_t>();
	for (ssize_t i = program.size() - 1; i >= 0; i--) {
		switch (program[i].com) {
			case JumpBwd: stack.push(i);
				break;
			case JumpFwd: program[i].data = stack.top();
				stack.pop();
				break;
			default: break;
		}
	}
}

void annotate_backward(std::vector<struct ins> &program) {
	auto stack = std::stack<ssize_t>();
	for (ssize_t i = 0; i < static_cast<ssize_t>(program.size()); i++) {
		switch (program[i].com) {
			case JumpFwd: stack.push(i);
				break;
			case JumpBwd: program[i].data = stack.top();
				stack.pop();
				break;
			default: break;
		}
	}
}

void annotate(std::vector<struct ins> &program) {
	annotate_forward(program);
	annotate_backward(program);
}

std::vector<struct ins> optimise(std::vector<struct ins> program) {
	auto result = std::vector<struct ins>();

	for (const auto ins : program) {
		switch (ins.com) {
			case OutCell:
			case InpCell:
			case JumpFwd:
			case JumpBwd:
				result.push_back(ins);
				continue;
			default: break;
		}
		if (result.empty() or result[result.size() - 1].com != ins.com) {
			result.push_back(ins);
			continue;
		}

		switch (ins.com) {
			case IncCell:
			case IncDptr:
				result[result.size() - 1].data++;
				break;
			case DecCell:
			case DecDptr:
				result[result.size() - 1].data--;
				break;
			default:
				throw "";
		}
	}

	return result;
}

std::vector<struct ins> compile(std::vector<char> source) {
	auto program = optimise(translate(source));
	annotate(program);
	return program;
}

int main(int argc, char *argv[]) {
	size_t dptr = 0;
	auto tape = std::array<uint_fast8_t, 30000>();

	if (argc != 2) {
		std::cerr << "Usage: " << argv[0] << " <SOURCE>" << std::endl;
		return 2;
	}

	auto t0 = std::chrono::steady_clock::now();

	auto source = read_file(argv[1]);

	auto t1 = std::chrono::steady_clock::now();

	auto program = compile(source);

	auto t2 = std::chrono::steady_clock::now();

	for (size_t iptr = 0; iptr < program.size(); iptr++) {
		const auto &ins = program[iptr];
		switch (ins.com) {
			case IncDptr:
			case DecDptr:
				dptr += ins.data;
				break;
			case IncCell:
			case DecCell:
				tape[dptr] += ins.data;
				break;
			case OutCell:
				putchar(tape[dptr]);
				break;
			case InpCell:
				tape[dptr] = getchar();
				break;
			case JumpFwd:
				if (tape[dptr] == 0)
					iptr = ins.data;
				break;
			case JumpBwd:
				if (tape[dptr] != 0)
					iptr = ins.data;
				break;
		}
	}

	auto t3 = std::chrono::steady_clock::now();

	std::cerr << "Time (μs)" << std::endl;
	std::cerr << "----------" << std::endl;
	std::cerr << "Reading  : " << std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count() << std::endl;
	std::cerr << "Compiling: " << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() << std::endl;
	std::cerr << "Executing: " << std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count() << std::endl;

	return 0;
}
