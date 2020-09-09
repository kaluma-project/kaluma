#!/usr/bin/env ruby

STDERR.puts "ARGV[0]: #{ARGV[0]}"

text = ''
File.open(ARGV[0]).read.each_line do |line|
	line.strip!
	line.gsub!(/"/, "'")
	line.gsub!(/\\/, "\\\\\\")
	line += '\n'
	text += line
end

name = File.basename(ARGV[0], ".*")
print "const char* const #{name}_script =\n\"#{text}\";\n"
