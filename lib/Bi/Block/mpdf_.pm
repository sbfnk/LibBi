=head1 NAME

mpdf_ - block for multivariate pdf actions.

=cut

package Bi::Block::mpdf_;

use parent 'Bi::Block';
use warnings;
use strict;

our $BLOCK_ARGS = [];

sub validate {
    my $self = shift;

    $self->process_args($BLOCK_ARGS);

    if (@{$self->get_blocks} > 0) {
        die("a 'mpdf_' block may not contain nested blocks\n");
    }
}

1;

=head1 AUTHOR

Sebastian Funk <sebastian.funk@lshtm.ac.uk>

=head1 VERSION

$Rev$ $Date$
