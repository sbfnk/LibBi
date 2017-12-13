=head1 NAME

mvgaussian - Gaussian distribution.

=head1 SYNOPSIS

    x ~ mbgaussian(mu, Sigma)

=head1 DESCRIPTION

A C<mvgaussian> action specifies that a variable is distributed
according to a multivariate gaussian with the given the given C<mean>
and C<cov> parameters.

=cut

package Bi::Action::mvgaussian;

use parent 'Bi::Action';
use warnings;
use strict;

=head1 PARAMETERS

=over 4

=item C<mean> (position 0)

Mean.

=item C<cov> (position 1)

Covariance.

=back

=cut
our $ACTION_ARGS = [
  {
    name => 'mean',
    positional => 1,
  },
  {
    name => 'cov',
    positional => 1,
  },
  {
    name => 'log',
    default => 0
  }
];

sub validate {
    my $self = shift;

    # HACK: change default of log argument according to class name -- this
    # gets inherited by Bi::Action::log_mbgaussian etc.
    $ACTION_ARGS->[2]->{default} = int(ref($self) =~ /^Bi\:\:Action\:\:log/);

    Bi::Action::validate($self);
    $self->process_args($ACTION_ARGS);
    $self->set_name('mvgaussian'); # synonyms exist, standardise name
    $self->ensure_op('~');
    $self->ensure_vector('mu');
    $self->ensure_matrix('cov');
    $self->ensure_scalar('log');
    $self->ensure_const('log');

    unless ($self->get_left->get_shape->compat($self->get_shape)) {
    	die("incompatible sizes on left and right sides of action.\n");
    }

    $self->set_parent('mpdf_');
    $self->set_can_combine(1);
    $self->set_unroll_args(0);
    $self->set_is_matrix(1);
}

1;

=head1 AUTHOR

Sebastian Funk <sebastian.funk@lshtm.ac.uk>

=head1 VERSION

$Rev$ $Date$
