/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adaza-ru <adaza-ru@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/28 18:26:06 by adaza-ru          #+#    #+#             */
/*   Updated: 2026/07/13 23:02:04 by adaza-ru         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "h_codexion.h"

int	main(int argc, char **argv)
{
	if (argc != 9 || check_args_errors(argv) == 1)
		return (1);
	parse_args();
	start_simulation();
	return (0);
}
