/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   arg_errors.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adaza-ru <adaza-ru@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/28 18:43:11 by adaza-ru          #+#    #+#             */
/*   Updated: 2026/09/05 19:58:14 by adaza-ru         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "h_codexion.h"

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
		return (1);
	while (i < 8)
	{
		if (check_format_error(argv[i]) || check_atoi_error(argv[i], i))
			return (i + 1);
		i++;
	}
	if (strcmp(argv[i], "fifo") && strcmp(argv[i], "edf"))
		return (i + 1);
	return (0);
}
