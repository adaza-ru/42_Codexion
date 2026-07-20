/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   errors.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adaza-ru <adaza-ru@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/28 18:43:11 by adaza-ru          #+#    #+#             */
/*   Updated: 2026/07/19 21:58:34 by adaza-ru         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "h_codexion.h"

void	print_error(int n)
{
	char	*error_messages[9];

	error_messages[0] = "Wrong number of coders. Ir must be a positive"
		" integer between 1 and 250.\n";
	error_messages[1] = "Wrong time to burnout. It must be "
		"a positive non-zero integer.\n";
	error_messages[2] = "Wrong time to compile. It must be a"
		"positive non-zero integer.\n";
	error_messages[3] = "Wrong time to debug. It must be a "
		"positive non-zero integer.\n";
	error_messages[4] = "Wrong time to refactor. It must be a "
		"positive non-zero integer.\n";
	error_messages[5] = "Wrong number of compiles. It must be a "
		"positive non-zero integer.\n";
	error_messages[6] = "Wrong dongle cooldown. It must be a "
		"positive non-zero integer.\n";
	error_messages[7] = "Wrong scheduler. It must \"edf\" or \"fifo\".\n";
	error_messages[8] = "Wrong arguments format. Usage: \"./codexion"
		"<number_of_coders> <time_to_burnout> <time_to_compile>\n "
		"<time_to_debug> <time_to_refactor> <number_of_compiles_required>"
		" <dongle_cooldown> <scheduler>\"\n";
	fprintf(stderr, DEATH_CLR "ERROR: %s" RESET, error_messages[n - 1]);
}

static int	check_atoi_error(char *str, int i)
{
	long long	n;
	int			sign;
	int			digits;

	n = 0;
	sign = 1;
	digits = 0;
	while (*str == ' ')
		str++;
	if (*str == '+')
		str++;
	while (*str >= '0' && *str <= '9')
	{
		n = n * 10 + (*str - '0');
		str++;
		if (++digits > 11)
			return (1);
	}
	if (digits == 0 || (*str && *str != ' '))
		return (1);
	n *= sign;
	if (i == 1)
		return (n < 1 || n > 250);
	return (n < 1 || n > 2147483647);
}

static int	check_format_error(char *arg)
{
	int	i;

	i = 0;
	if (arg[i] == '\0')
		return (0);
	while (arg[i] == ' ')
		i++;
	if (arg[i] == '+')
		i++;
	while (arg[i])
	{
		if (!(arg[i] >= '0' && arg[i] <= '9'))
			return (1);
		i++;
	}
	return (0);
}

int	check_arg_errors(int argc, char **argv)
{
	int	i;

	i = 1;
	if (argc != 9)
		return (9);
	while (i < 8)
	{
		if (check_format_error(argv[i]) || check_atoi_error(argv[i], i))
			return (i);
		i++;
	}
	if (strcmp(argv[i], "fifo") && strcmp(argv[i], "edf"))
		return (i);
	return (0);
}
