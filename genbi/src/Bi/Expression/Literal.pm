=head1 NAME

Bi::Expression::Literal - numeric literal.

=head1 SYNOPSIS

    use Bi::Expression::Literal;

=head1 METHODS

=over 4

=cut

package Bi::Expression::Literal;

use base 'Bi::Expression';
use warnings;
use strict;

use Carp::Assert;

=item B<new>(I<value>)

Constructor.

=cut
sub new {
    my $class = shift;
    my $value = shift;

    my $self = {
        _value => $value
    };
    bless $self, $class;
    
    return $self;
}

=item B<clone>

Return a clone of the object.

=cut
sub clone {
    my $self = shift;
    
    my $clone = { %$self };
    bless $clone, ref($self);
    
    return $clone; 
}

=item B<get_value>

Get the literal value.

=cut
sub get_value {
    my $self = shift;
    return $self->{_value};
}

=item B<num_dims>

Get the dimensionality of the expression.

=cut
sub num_dims {
    return 0;
}

=item B<accept>(I<visitor>, ...)

Accept visitor.

=cut
sub accept {
    my $self = shift;
    my $visitor = shift;
    my @args = @_;
    
    return $visitor->visit($self, @args);
}

=item B<equals>(I<obj>)

Does object equal I<obj>?

=cut
sub equals {
    my $self = shift;
    my $obj = shift;
    
    return (
        ref($obj) eq ref($self) &&
        $self->get_value == $obj->get_value); 
}

1;

=back

=head1 SEE ALSO

L<Bi::Expression>

=head1 AUTHOR

Lawrence Murray <lawrence.murray@csiro.au>

=head1 VERSION

$Rev$ $Date$
