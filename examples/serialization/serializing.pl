use lib qw(..);

use JSON qw( );

my $filename = 'generated (1).json';

my $json_text = do {
   open(my $json_fh, "<:encoding(UTF-8)", $filename)
      or die("Can't open \$filename\": $!\n");
   local $/;
   <$json_fh>
};

my $json = JSON->new;
my $data = $json->decode($json_text);

my $json_text = JSON->new->utf8->encode($data);

