#!/usr/bin/env perl
# vim:ts=4:sw=4:noet
use strict;
use HTML::Entities; # from libwww-perl (debian?) package

my $file = '../changelog.txt';
my $out = 'changelog.html';

rename $out, $out.'.old' or die;

my @data;

open IN, '<'.$file or die;
while(<IN>) {
	# remove CRLF
	chomp; s/\r$//;
	if( /^[[:blank:]]*$/ ) {
		# pass
	} elsif( /^[[:blank:]]*\-\-[[:blank:]]+(.*?)[[:blank:]]+\-\-[[:blank:]]*$/ ) {
		# new paragraph
		my $version = $1;
		my @paragraph = ($version);
		push @data, [@paragraph];
	} elsif( /^\*[[:blank:]]*(.*?)[[:blank:]]*$/ ) {
		# new item
		my $item = $1;
		push @{$data[$#data]}, $item;
	} elsif( /^[[:blank:]]+(.*?)[[:blank:]]*$/ ) {
		# continuation of previous item
		my $item = $1;
		my @a = @{$data[$#data]};
		$a[$#a] .= ' '.$item;
		@{$data[$#data]} = @a;
	} elsif( /^[[:blank:]]*(.*?)[[:blank:]]*$/ ) {
		# matches anything
		my $item = $1;
		push @{$data[$#data]}, "-- $item --"; # unknown format, add markers
	}	
}
close IN;

open OUT, '>'.$out or die;

print OUT "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\r
<html>\r
<head>\r
  <meta content=\"en-us\" http-equiv=\"Content-Language\">\r
  <meta http-equiv=\"content-type\" content=\"text/html;charset=utf-8\">\r
  <link href=\"office11.css\" rel=\"stylesheet\" type=\"text/css\">\r
  <title>Changelog</title>\r
  <style type=\"text/css\">\r
    li { margin-left: auto; margin: 0em 0em 0em 0em; }\r
  </style>\r
</head>\r
<body>\r
<h1>DC++ Changelog</h1>\r
See the version history of DC++ below.\r
\r
";

for my $paragraph (@data) {
	my $head = shift @{$paragraph};
	$head =~ /^([0-9.]+) ([0-9-]+)$/
		and print OUT '<h2>'.$1.' <span style="color: gray;">('.$2.")</span></h2>\r\n"
		or  print OUT '<h2>'.encode_entities($head)."</h2>\r\n";
	print OUT "<ul>\r\n";
	for my $item (@{$paragraph}) {
		print OUT '  <li>'.encode_entities($item)."</li>\r\n";
	}
	print OUT "</ul>\r\n";
	print OUT "\r\n";
}

print OUT "</body>\r\n";
print OUT "</html>\r\n";
close OUT;
